#include "vehicle_inputs.h"

#include <Arduino.h>

#include "config.h"

namespace VehicleInputs {
namespace {

bool lastRawAcc = false;
bool stableAcc = false;
uint32_t rawAccChangedAt = 0;

uint16_t batteryAdcRaw = 0;
uint32_t batteryAdcMillivolts = 0;
float batteryVoltage = 0.0F;

uint32_t batterySampleSumMillivolts = 0;
uint8_t batterySampleCount = 0;
uint32_t lastBatterySampleAt = 0;
bool batteryReadingReady = false;

bool readRawAcc()
{
    return digitalRead(MainConfig::Pin::ACC) ==
           MainConfig::Logic::ACC_ON;
}

void sampleBatteryVoltage()
{
    batteryAdcRaw =
        analogRead(MainConfig::Pin::BATTERY_ADC);

    const uint32_t sampleMillivolts =
        analogReadMilliVolts(
            MainConfig::Pin::BATTERY_ADC);

    batterySampleSumMillivolts +=
        sampleMillivolts;
    ++batterySampleCount;

    if (batterySampleCount <
        MainConfig::Timing::
            BATTERY_FILTER_SAMPLE_COUNT) {
        return;
    }

    batteryAdcMillivolts =
        batterySampleSumMillivolts /
        batterySampleCount;

    const float adcVoltage =
        batteryAdcMillivolts / 1000.0F;

    const float dividerRatio =
        (MainConfig::Battery::R_TOP_OHM +
         MainConfig::Battery::R_BOTTOM_OHM) /
        MainConfig::Battery::R_BOTTOM_OHM;

    batteryVoltage =
        adcVoltage *
        dividerRatio *
        MainConfig::Battery::
            CALIBRATION_FACTOR;

    batterySampleSumMillivolts = 0;
    batterySampleCount = 0;
    batteryReadingReady = true;
}

}  // namespace

void begin()
{
    /*
     * GPIO34 và GPIO35 là ngõ vào.
     * Hai đường này đã có mạch ngoài trên PCB.
     */
    pinMode(MainConfig::Pin::ACC, INPUT);
    pinMode(MainConfig::Pin::BATTERY_ADC, INPUT);

    analogReadResolution(12);

    /*
     * Cho phép đo dải điện áp ADC cao hơn mức mặc định.
     */
    analogSetPinAttenuation(
        MainConfig::Pin::BATTERY_ADC,
        ADC_11db);

    lastRawAcc = readRawAcc();
    stableAcc = lastRawAcc;
    rawAccChangedAt = millis();

    batteryAdcRaw = 0;
    batteryAdcMillivolts = 0;
    batteryVoltage = 0.0F;
    batterySampleSumMillivolts = 0;
    batterySampleCount = 0;
    lastBatterySampleAt = 0;
    batteryReadingReady = false;
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

    if (now - lastBatterySampleAt >=
        MainConfig::Timing::
            BATTERY_SAMPLE_INTERVAL_MS) {
        lastBatterySampleAt = now;
        sampleBatteryVoltage();
    }
}

bool isAccOn()
{
    return stableAcc;
}

uint16_t getBatteryAdcRaw()
{
    return batteryAdcRaw;
}

uint32_t getBatteryAdcMillivolts()
{
    return batteryAdcMillivolts;
}

float getBatteryVoltage()
{
    return batteryVoltage;
}

bool isBatteryReadingReady()
{
    return batteryReadingReady;
}

}  // namespace VehicleInputs
