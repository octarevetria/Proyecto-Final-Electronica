#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include "esp_bt.h"
#include "PMUManager.h"

#define LORA_SS 18
#define LORA_RST 23
#define LORA_DI0 26
#define GPS_RX 34
#define GPS_TX 12
#define GPS_BAUD 9600
#define LORA_FREQ 866E6
#define MESSAGE_COUNT_BEFORE_SLEEP 10  // Número de mensajes antes de entrar en modo deep sleep
#define TIME_COUNT_BEFORE_SLEEP 60E3  // Duración del ciclo en milisegundos (1 min)
#define SLEEP_DURATION 60E6           // Duración del sueño en microsegundos (1 min)

extern bool pmuInterrupt;  // Declare the external variable

static void setPmuFlag() {
  pmuInterrupt = true;
}

void setupLoRa();
void setupGPS();
void enterDeepSleep();

PMUManager PMU(Wire, 21, 22, 0x34);
TinyGPSPlus gps;

int tracker_id = 3;
int messageCount = 0;
double latitude = 0.0;
double longitude = 0.0;
int satelliteCount = 0;
int bateria = 0;
int endPacketStatus = 0;
bool loraListo = false;

void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  SPI.begin();
  PMU.setup();

  // Apago WiFi y Bluetooth para disminuir consumo
  WiFi.disconnect(true);
  btStop();

  setupGPS();
}

void loop() {
  Serial.print("esta Cargando:");
  Serial.println(PMU.isCharging() ? "SI" : "NO");

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }
  // Obtener los datos de ubicación
  double latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
  double longitude = gps.location.isValid() ? gps.location.lng() : 0.0;
  int satelliteCount = gps.satellites.isValid() ? gps.satellites.value() : 0;
  int bateria = PMU.getBatteryPercent();

  latitude = 33.2024; // Van harcodeados porque no quiero que gps se vuelva a prender
  longitude = 33.2024; // Idem

  if (latitude != 0 && longitude != 0) {
    if (loraListo == false) {
      // Apagar GPS
      Serial.println("Info valida de GPS");
      PMU.disableALDO3();
      // Encender LoRa
      PMU.setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
      PMU.enablePowerOutput(XPOWERS_ALDO2);
      setupLoRa();
      loraListo = true;
    }
    // Crear un payload con formato designado
    String payload = "{lat:" + String(latitude, 6) + ",lon:" + String(longitude, 6) + ",sat:" + String(satelliteCount) + ",trk:" + String(tracker_id) + ",bat:" + String(bateria) + "}";

    // Depuración: Mostrar el mensaje que se va a enviar
    Serial.print("Preparando para enviar: ");
    Serial.println(payload);

    // Enviar toda la información a través de LoRa
    LoRa.beginPacket();
    LoRa.print(payload);

    // Depuración: Verificar si se está iniciando el paquete
    Serial.println("Paquete iniciado");

    endPacketStatus = LoRa.endPacket();

    // Depuración: Confirmar que el paquete ha sido enviado
    if (endPacketStatus == 1) {
      messageCount++;
      Serial.print("Paquete enviado numero ");
      Serial.println(messageCount);
    } else {
      Serial.print("Error al enviar el paquete numero ");
      Serial.println(messageCount);
    }

    // Mostrar lo que se envio en el monitor serial
    Serial.print("Lo que se envio: ");
    Serial.println(payload);
  } else 
  {
    Serial.println("No hay datos validos para enviar, espere...");
  }
  if (messageCount >= MESSAGE_COUNT_BEFORE_SLEEP) // Tras 10 intentos de envío se duerme
  {
    Serial.print("Se alcanzaron los ");
    Serial.print(MESSAGE_COUNT_BEFORE_SLEEP);
    Serial.println(" intentos de envío máximos");
    enterDeepSleep();
  }
  if (timeAwake()) // Si transcurren más de 2 minutos desde que se desperto, se vuelve a dormir
  {
    Serial.print("Se alcanzaron los ");
    //Tiempo en minutos
    Serial.print(TIME_COUNT_BEFORE_SLEEP / 60E3);
    Serial.println(" minutos de encendido máximo");
    enterDeepSleep();
  }
  delay(5000);  // Esperar 5 segundos antes de enviar nuevamente
}

void setupLoRa()
{
  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, HIGH);
  pinMode(LORA_DI0, INPUT);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  while (!LoRa.begin(LORA_FREQ)) 
  {
    Serial.println("Iniciación de LoRa fallida!");
    delay(200);
  }
  LoRa.setTxPower(20);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(9);
  LoRa.setCodingRate4(6);
  LoRa.setPreambleLength(10);
  Serial.println("LoRa inicializado correctamente.");
}

void setupGPS() {
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS inicializado correctamente.");
}

bool timeAwake() 
{
  bool salida = false;
  if (millis() > TIME_COUNT_BEFORE_SLEEP) {
    salida = true;
  }
  return salida;
}

void enterDeepSleep() 
{
  Serial.println("Entrando en modo de ahorro de energía durante 1 minuto ...");

  // Configurar el tiempo de sueño
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);

  // Apagar periféricos
  LoRa.sleep();                           // Apagar LoRa
  PMU.disablePowerOutput(XPOWERS_ALDO2);  // Apagar ALDO2 LoRa
  PMU.disablePowerOutput(XPOWERS_ALDO3);  // Apagar ALDO3 GPS

  // Entrar en sueño profundo
  esp_deep_sleep_start();
}