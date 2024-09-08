// Only supports SX1276/SX1278
#include <LoRa.h>
#include "LoRaBoards.h"

#define TIME_COUNT_BEFORE_SLEEP 180E3   // Duración del ciclo en milisegundos (3 min)
#define SLEEP_DURATION 300E6            // Duración del sueño en microsegundos (5 min)

int counter = 0;

void setup() {
  setupBoards();
  // When the power is turned on, a delay is required.
  delay(1500);

#ifdef RADIO_TCXO_ENABLE
  pinMode(RADIO_TCXO_ENABLE, OUTPUT);
  digitalWrite(RADIO_TCXO_ENABLE, HIGH);
#endif

  Serial.println("LoRa Sender");
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(LORA_FREQ_CONFIG)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  else{
    Serial.println("Starting LoRa correctly!");
  }
  LoRa.setSignalBandwidth(125E3);
  LoRa.setSpreadingFactor(10);
  LoRa.setCodingRate4(5);
}

void loop() {
  Serial.print("getBatteryPercent:");
  Serial.print(PMU->getBatteryPercent());
  Serial.println("%");
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();


  counter++;

  if (millis() > TIME_COUNT_BEFORE_SLEEP) {
    enterDeepSleep();
  }

  delay(5000);
}

void enterDeepSleep() {
  Serial.println("Entrando en modo de ahorro de energía durante 60 minutos ...");

  // Configurar el tiempo de sueño
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);

  // Para disminuir el consumo se desactivan perisfericos
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

  // Apagar periféricos
  LoRa.sleep();                           // Apagar LoRa
  //PMU.disablePowerOutput(XPOWERS_ALDO2);  // Apagar ALDO2 LoRa
  //PMU.disablePowerOutput(XPOWERS_ALDO3);  // Apagar ALDO3 GPS

  // Entrar en sueño profundo
  esp_deep_sleep_start();
}