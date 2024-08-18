#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include "PMUManager.h"

#define LORA_SS 18
#define LORA_RST 23
#define LORA_DI0 26
#define GPS_RX 34
#define GPS_TX 12
#define GPS_BAUD 9600
#define LORA_FREQ 868.0
#define MESSAGE_COUNT_BEFORE_SLEEP 10  // Número de mensajes antes de entrar en modo de ahorro de energía
#define SLEEP_DURATION 3600e6          // Duración del sueño en microsegundos (1 hora)

extern bool pmuInterrupt;  // Declare the external variable

static void setPmuFlag() {
  pmuInterrupt = true;
}

void setupLoRa();
void setupGPS();
void enterDeepSleep();

PMUManager PMU(Wire, 21, 22, 0x34);
TinyGPSPlus gps;

char tracker_id = 3;
int messageCount = 0;
double latitude = 0.0;
double longitude = 0.0;
int satelliteCount = 0;
char bateria = 69;
int endPacketStatus = 0;
bool loraListo = false;

void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  SPI.begin();
  PMU.setup();
  setupGPS();
}

void loop() {

  //// MSJS de Depuracion /////
  Serial.print("isCharging:");
  Serial.println(PMU.isCharging() ? "YES" : "NO");
  //////////////////////////////////////////////////////
  if (latitude == 0 && longitude == 0) {
    while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
    }

    // Obtener los datos de ubicación
    double latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
    double longitude = gps.location.isValid() ? gps.location.lng() : 0.0;
    int satelliteCount = gps.satellites.isValid() ? gps.satellites.value() : 0;
    int bateria = PMU.getBatteryPercent();
  }

  if (latitude != 0 && longitude != 0) {
    if (loraListo == false) {
      //Apagar GPS
      PMU.disableALDO3();
      //Encender LoRa
      PMU.setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
      PMU.enablePowerOutput(XPOWERS_ALDO2);
      setupLoRa();
      loraListo = true;
    }

    // Crear un mensaje para enviar
    String message = "lat:" + String(latitude, 6) + ",lon:" + String(longitude, 6) + ",sat:" + String(satelliteCount) + ",trk:" + String(tracker_id) ",bat:" + String(bateria);

    // Depuración: Mostrar el mensaje que se va a enviar
    Serial.print("Preparando para enviar: ");
    Serial.println(message);

    // Enviar los datos de GPS a través de LoRa
    LoRa.beginPacket();
    LoRa.print(message);

    // Depuración: Verificar si se está iniciando el paquete
    Serial.println("Paquete iniciado");

    endPacketStatus = LoRa.endPacket();

    // Depuración: Confirmar que el paquete ha sido enviado
    if (endPacketStatus == 1) {
      Serial.println("Paquete enviado");
      messageCount++;
    } else {
      Serial.println("Error al enviar el paquete");
    }

    // Mostrar los datos en el monitor serie
    Serial.println("Enviado: " + message);
  }
  if (messageCount >= MESSAGE_COUNT_BEFORE_SLEEP) {
    //Entramos en modo ahorro de energia
    enterDeepSleep();
  }

  Serial.println("No hay datos validos para enviar, espere...");
  delay(5000);  // Esperar 5 segundos antes de enviar nuevamente
}

void setupLoRa() {
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, HIGH);
  pinMode(LORA_DI0, INPUT);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Iniciación de LoRa fallida!");
    while (1)
      ;
  }
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(10);
  LoRa.setCodingRate4(5);
  Serial.println("LoRa inicializado correctamente.");
}

void setupGPS() {
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS inicializado correctamente.");
}

void enterDeepSleep() {
  Serial.println("Entrando en modo de ahorro de energía durante 1 hora...");

  // Configurar el tiempo de sueño
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);

  // Apagar periféricos
  LoRa.sleep();                           // Apagar LoRa
  PMU.disablePowerOutput(XPOWERS_ALDO2);  // Apagar ALDO2 LoRa
  PMU.disablePowerOutput(XPOWERS_ALDO3);  // Apagar ALDO3 GPS

  // Entrar en sueño profundo
  esp_deep_sleep_start();
}