#include <TimeLib.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include "PMUManager.h"

#define LORA_SS 18
#define LORA_RST 23
#define LORA_DI0 26
#define S232_RX 35
#define S232_TX 32
#define MSJ_BAUD 115200
#define LORA_FREQ 868.0
#define MESSAGE_COUNT_BEFORE_SLEEP 10  // Número de mensajes antes de entrar en modo de ahorro de energía
#define SLEEP_DURATION 3600e6          // Duración del sueño en microsegundos (1 hora)

extern bool pmuInterrupt;  // Declare the external variable

static void setPmuFlag() {
  pmuInterrupt = true;
}

PMUManager PMU(Wire, 21, 22, 0x34);

char tracker_id = 2;
const int messageInterval = 30000;
int messageCount = 0;
bool mandar_mensaje = 0;
double latitude = 51.001;
double longitude = 52.001;
char satelliteCount = 33;
char bateria = 66;
unsigned long lastReceiveTime = 0;
String mensaje_lora = "";
const unsigned long receiveTimeout = 10000;  // 10 segundos

/*
WiFiClient wificlient;
//Para ThingsBoard
const char* token = "Vm8OAEIC2OTn2ESgekJi";             // Token de dispositivo
const char* thingsboardServer = "demo.thingsboard.io";  // url a host server de ThingsBoard
const char* headerTelemetry = "v1/devices/me/telemetry";      
const char* headerAttribute = "v1/devices/me/attributes";    
// Datos Wifi
const char* ssid = "Palmasola";
const char* password = "RomaBerlinTokyo2934";
*/

void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  SPI.begin();
  PMU.setup();
  Serial2.begin(MSJ_BAUD, SERIAL_8N1, S232_RX, S232_TX);
  //Encender LoRa
  PMU.setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
  PMU.enablePowerOutput(XPOWERS_ALDO2);
  setupLoRa();
}

void loop() {
  int buffer = LoRa.parsePacket();
  Serial.println(buffer);
  if (buffer != 0) {
    String mensaje_lora = "";
    while (LoRa.available()) {
      Serial.println("En while de LoRa");
      mensaje_lora += (char)LoRa.read();
      mandar_mensaje = 1;
    }
    lastReceiveTime = millis();
  }
  if (millis() - lastReceiveTime > receiveTimeout) {
    Serial.println("Tiempo de espera superado. Reiniciando LoRa...");
    ESP.restart();
    LoRa.end();
    if (!LoRa.begin(LORA_FREQ)) {
      Serial.println("Reinicialización de LoRa fallida!");
      while (1)
        ;
    }
    lastReceiveTime = millis();
  }
  char mensaje[50] = "";
  int largo = snprintf(mensaje, 50, "{lat:%lf,lon:%lf,sat:%d,trk:%d,bat:%d}", latitude, longitude, satelliteCount, tracker_id, bateria);
  Serial.print("Mensaje fixado: ");
  Serial.println(mensaje);
  Serial.print("Mensaje LoRa: ");
  Serial.println(mensaje_lora);
  Serial2.println(mensaje);

  delay(10000);  // Esperar 10 segundos
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
