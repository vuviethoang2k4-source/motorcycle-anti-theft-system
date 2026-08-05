#pragma once

#include "protocol.h"

/*
 * Các đầu vào có thể kích hoạt báo động.
 * Mốc 4 mới sử dụng accOn.
 * motionDetected sẽ được dùng khi thêm MPU6050.
 */
struct AlarmInputs {
    bool accOn = false;
    bool motionDetected = false;
};

struct SystemStatus {
    SystemState state = SystemState::DISARMED;
    bool accOn = false;
    bool motionDetected = false;
    bool gpsAvailable = false;
    float batteryVoltage = 0.0F;
};
