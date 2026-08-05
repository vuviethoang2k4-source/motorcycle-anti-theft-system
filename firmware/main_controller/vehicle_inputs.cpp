#include "vehicle_inputs.h"
#include <Arduino.h>
#include "config.h"

namespace VehicleInputs {
namespace {
bool lastRawAcc = false;
bool stableAcc = false;
uint32_t rawAccChangedAt = 0;
uint16_t batteryAdcRaw = 0;

bool readRawAcc()
{
    return digitalRead(MainConfig::Pin::ACC) ==
           MainConfig::Logic::ACC_ON;
}
}

void begin()
{
    pinMode(MainConfig::Pin::ACC, INPUT);
    pinMode(MainConfig::Pin::BATTERY_ADC, INPUT);

    analogReadResolution(12);

    lastRawAcc = readRawAcc();
    stableAcc = lastRawAcc;
    rawAccChangedAt = millis();
    batteryAdcRaw = analogRead(MainConfig::Pin::BATTERY_ADC);
}

void update()
{
    const uint32_t now = millis();
    const bool currentRawAcc = readRawAcc();

    if (currentRawAcc != lastRawAcc) {
        lastRawAcc = currentRawAcc;
        rawAccChangedAt = now;
    }

    if (now - rawAccChangedAt >=
        MainConfig::Timing::ACC_DEBOUNCE_MS) {
        stableAcc = lastRawAcc;
    }

    batteryAdcRaw = analogRead(MainConfig::Pin::BATTERY_ADC);
}

bool isAccOn() { return stableAcc; }
uint16_t getBatteryAdcRaw() { return batteryAdcRaw; }

}
