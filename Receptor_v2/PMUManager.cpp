#include "PMUManager.h"


PMUManager::PMUManager(TwoWire &wire, uint8_t sda, uint8_t scl, uint8_t addr)
  : XPowersAXP2101(wire, sda, scl, addr) {}

void PMUManager::setup() {
  begin(Wire, 0x34, 21, 22);
  setPowerChannelVoltage(XPOWERS_VBACKUP, 3300);
  enablePowerOutput(XPOWERS_VBACKUP);
  setProtectedChannel(XPOWERS_DCDC1);

  setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
  enablePowerOutput(XPOWERS_ALDO2);

  setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
  enablePowerOutput(XPOWERS_ALDO3);
  setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

  enableBattDetection();
  enableVbusVoltageMeasure();
  enableBattVoltageMeasure();
  enableSystemVoltageMeasure();

  pinMode(35, INPUT_PULLUP);
 
}
