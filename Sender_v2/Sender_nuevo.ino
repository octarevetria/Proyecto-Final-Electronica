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
#define LORA_FREQ 866E6


extern bool pmuInterrupt;  // Declare the external variable

static void setPmuFlag() {
  pmuInterrupt = true;
}

void setupLoRa();
void setupGPS();


PMUManager PMU(Wire, 21, 22, 0x34);
TinyGPSPlus gps;
int tracker_id = 3;
int messageCount = 0;
double latitude = 0.0;
double longitude = 0.0;
int satelliteCount = 0;
int bateria = 69;
int endPacketStatus = 0;
bool loraListo = false;



void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  SPI.begin();
  PMU.setup();
  delay(100);
  setupLoRa();
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
  // Crear un mensaje para enviar
  String message = "{lat:" + String(latitude, 6) + ",lon:" + String(longitude, 6) + ",sat:" + String(satelliteCount) + ",trk:" + String(tracker_id) + ",bat:" + String(bateria) + "}";


  // Depuración: Mostrar el mensaje que se va a enviar
  Serial.print("Preparando para enviar: ");
  Serial.println(message);

  // Enviar los datos de GPS a través de LoRa
  LoRa.beginPacket();
  LoRa.print(message);

  // Depuración: Verificar si se está iniciando el paquete
  Serial.println("Paquete iniciado");

  int endPacketStatus = LoRa.endPacket();

  // Depuración: Confirmar que el paquete ha sido enviado
  if (endPacketStatus == 1) {
    Serial.println("Paquete enviado");
  } else {
    Serial.println("Error al enviar el paquete");
  }

  // Mostrar los datos en el monitor serie
  Serial.println("Enviado: " + message);

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