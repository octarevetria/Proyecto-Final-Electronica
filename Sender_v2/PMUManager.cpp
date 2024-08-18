#include "PMUManager.h"

bool pmuInterrupt;

static void setPmuFlag() {
  pmuInterrupt = true;
}

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

  // Set the precharge charging current
  setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
  // Set constant current charge current limit
  setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_200MA);
  // Set stop charging termination current
  setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);

  // Set charge cut-off voltage
  setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V1);

  disableTSPinMeasure();
  // Set the minimum common working voltage of the PMU VBUS input,
  // below this value will turn off the PMU
  setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);

  // Set the maximum current of the PMU VBUS input,
  // higher than this value will turn off the PMU
  setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);

  pinMode(35, INPUT_PULLUP);
  attachInterrupt(35, setPmuFlag, FALLING);
}
