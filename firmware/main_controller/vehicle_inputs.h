#pragma once

#include <Arduino.h>

/*
 * Quản lý các tín hiệu lấy từ xe:
 * - ACC qua PC817;
 * - điện áp ắc quy qua cầu phân áp vào GPIO35.
 */
namespace VehicleInputs {

void begin();
void update();

bool isAccOn();

uint16_t getBatteryAdcRaw();
uint32_t getBatteryAdcMillivolts();

/*
 * Điện áp ắc quy đã:
 * - quy đổi theo cầu phân áp;
 * - lấy trung bình nhiều mẫu;
 * - nhân hệ số hiệu chỉnh.
 */
float getBatteryVoltage();

bool isBatteryReadingReady();

}  // namespace VehicleInputs
