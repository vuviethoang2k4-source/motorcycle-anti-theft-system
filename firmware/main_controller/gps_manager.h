#pragma once

#include <Arduino.h>

/*
 * Module GPS:
 * - dùng UART1 của ESP32;
 * - RX ESP32 GPIO13 nhận từ TX GPS;
 * - TX ESP32 GPIO12 gửi tới RX GPS;
 * - phân tích dữ liệu NMEA bằng TinyGPSPlus.
 */
namespace GpsManager {

bool begin();
void update();

bool isReceivingData();
bool hasValidFix();

double getLatitude();
double getLongitude();
float getSpeedKmph();

uint32_t getSatelliteCount();
uint32_t getLocationAgeMs();
uint32_t getCharactersProcessed();

/*
 * Chuỗi này sẽ được sms_manager sử dụng ở mốc sau.
 * Khi chưa có fix, hàm trả về "GPS: NO FIX".
 */
String getGoogleMapsUrl();

}  // namespace GpsManager
