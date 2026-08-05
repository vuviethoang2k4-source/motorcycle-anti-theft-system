#pragma once
#include <Arduino.h>

namespace VehicleInputs {
void begin();
void update();
bool isAccOn();
uint16_t getBatteryAdcRaw();
}
