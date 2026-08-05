#pragma once
#include "protocol.h"

struct SystemStatus {
    SystemState state = SystemState::DISARMED;
    bool accOn = false;
    bool motionDetected = false;
    bool gpsAvailable = false;
    float batteryVoltage = 0.0F;
};
