#include "PMUManager.h"
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <esp_now.h>
#include <WiFi.h>

#define MSJ_BAUD 115200
#define LORA_SS 18
#define LORA_RST 23
#define LORA_DI0 26
#define LORA_FREQ 866E6

uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

bool pmuInterrupt;
int packetCounter = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print(F("\r\n Master packet sent:\t"));
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void setupLoRa();

PMUManager PMU(Wire, 21, 22, 0x34);

void setup() {
  Wire.begin(21, 22);
  delay(100);
  Serial.begin(115200);
  SPI.begin();
  PMU.setup();
  delay(100);
  setupLoRa();
  // Setteo WiFi para esp_now
  WiFi.mode(WIFI_STA);
  // inicio esp_now
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("Error initializing ESP-NOW"));
    return;
  }
  // Declaro el send de esp_now
  esp_now_register_send_cb(OnDataSent);
  // Registro peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Lo agrego
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println(F("Failed to add peer"));
    ESP.restart();
    return;
  }
}

void loop() {
  delay(500);
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Valor de packetSize: ");
    Serial.println(packetSize);
    char received[packetSize + 1];
    int i = 0;
    while (LoRa.available()) {
      received[i++] = (char)LoRa.read();
    }
    received[i] = '\0';
    packetCounter++;
    delay(100);
    Serial.print("Recibido (Paquete ");
    Serial.print(packetCounter);
    Serial.print("): ");
    Serial.println(received);
    const char *datos_string = received;
    //Send message via ESP-NOW
    Serial.print("Lo que mando por esp_now: ");
    Serial.println(datos_string);
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)datos_string, strlen(datos_string));
    Serial.print("Resultado esp_now send: ");
    Serial.println(result);
    if (result) {
      ESP.restart();
    }
  }
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
