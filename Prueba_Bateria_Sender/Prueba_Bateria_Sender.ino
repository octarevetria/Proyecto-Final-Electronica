#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <WiFi.h>
//#include <BluetoothSerial.h>
//#include <BLEDevice.h>
#include "PMUManager.h"

#define LORA_SS 18
#define LORA_RST 23
#define LORA_DI0 26
#define GPS_RX 34
#define GPS_TX 12
#define GPS_BAUD 9600
#define LORA_FREQ 866E6
#define MESSAGE_COUNT_BEFORE_SLEEP 100  // Número de mensajes antes de entrar en modo deep sleep
#define TIME_COUNT_BEFORE_SLEEP 360E3   // Duración del ciclo en milisegundos (6 min)
#define SLEEP_DURATION 300E6            // Duración del sueño en microsegundos (5 min)

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
  WiFi.mode(WIFI_OFF);
  //WiFi.end();
  // Disable Bluetooth
  //BluetoothSerial SerialBT;
  //SerialBT.end();
  setupGPS();
}

void loop() {
  Serial.print("isCharging:");
  Serial.println(PMU.isCharging() ? "YES" : "NO");

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }
  // Obtener los datos de ubicación
  double latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
  double longitude = gps.location.isValid() ? gps.location.lng() : 0.0;
  int satelliteCount = gps.satellites.isValid() ? gps.satellites.value() : 0;
  int bateria = PMU.getBatteryPercent();

  latitude = 33.2024;
  longitude = 33.2024;

  if (latitude != 0 && longitude != 0) {
    if (loraListo == false) {
      //Apagar GPS
      Serial.println("Info valida de GPS");
      PMU.disableALDO3();
      //Encender LoRa
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
      Serial.println("Error al enviar el paquete");
    }

    // Mostrar lo que se envio en el monitor serial
    Serial.print("Lo que se envio: ");
    Serial.println(payload);
  } else {
    Serial.println("No hay datos validos para enviar, espere...");
  }
  if (messageCount >= MESSAGE_COUNT_BEFORE_SLEEP) {
    //Entramos en modo ahorro de energia
    Serial.print("Se realizaron ");
    Serial.print(MESSAGE_COUNT_BEFORE_SLEEP);
    Serial.println(" intentos de envio");
    Serial.println("Se alcanzo el numero maximo de envios");
    enterDeepSleep();
  }
  if (timeAwake()) {
    Serial.print("Se excedieron los ");
    //Tiempo en minutos
    Serial.print(TIME_COUNT_BEFORE_SLEEP / 60E3);
    Serial.println(" minutos encendido");
    Serial.println("Se alcanzo el tiempo maximo de encendido");
    enterDeepSleep();
  }
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

bool timeAwake() {
  bool salida = false;
  if (millis() > TIME_COUNT_BEFORE_SLEEP) {
    salida = true;
  }
  return salida;
}

void enterDeepSleep() {
  Serial.println("Entrando en modo de ahorro de energía durante 60 minutos ...");

  // Configurar el tiempo de sueño
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);

  // Apagar periféricos
  LoRa.sleep();                           // Apagar LoRa
  PMU.disablePowerOutput(XPOWERS_ALDO2);  // Apagar ALDO2 LoRa
  PMU.disablePowerOutput(XPOWERS_ALDO3);  // Apagar ALDO3 GPS

  // Entrar en sueño profundo
  esp_deep_sleep_start();
}