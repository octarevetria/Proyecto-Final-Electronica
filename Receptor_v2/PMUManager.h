#ifndef PMU_MANAGER_H
#define PMU_MANAGER_H

#include <XPowersLib.h>
#include <Wire.h>

class PMUManager : public XPowersAXP2101 {
public:
    PMUManager(TwoWire &wire, uint8_t sda, uint8_t scl, uint8_t addr);
    void setup();

    using XPowersAXP2101::disablePowerOutput;  // Make protected methods public
    using XPowersAXP2101::enablePowerOutput;
    using XPowersAXP2101::setPowerChannelVoltage;
    using XPowersAXP2101::setProtectedChannel;
};

extern bool pmuInterrupt; // Declare the external variable
  #endif // PMU_MANAGER_H
