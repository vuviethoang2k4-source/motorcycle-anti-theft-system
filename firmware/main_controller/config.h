#pragma once

#include <Arduino.h>

namespace MainConfig {

namespace Pin {

constexpr uint8_t GPS_TX = 12;
constexpr uint8_t GPS_RX = 13;

constexpr uint8_t GSM_RX = 16;
constexpr uint8_t GSM_TX = 17;

constexpr uint8_t MPU_SDA = 21;
constexpr uint8_t MPU_SCL = 22;

constexpr uint8_t STARTER_RELAY = 25;
constexpr uint8_t TURN_SIGNAL = 32;
constexpr uint8_t SIREN = 33;

constexpr uint8_t ACC = 34;
constexpr uint8_t BATTERY_ADC = 35;

}  // namespace Pin

namespace Logic {

constexpr uint8_t STARTER_LOCK_ACTIVE = HIGH;
constexpr uint8_t STARTER_LOCK_INACTIVE = LOW;

constexpr uint8_t SIREN_ACTIVE = HIGH;
constexpr uint8_t SIREN_INACTIVE = LOW;

constexpr uint8_t TURN_SIGNAL_ACTIVE = HIGH;
constexpr uint8_t TURN_SIGNAL_INACTIVE = LOW;

constexpr uint8_t ACC_ON = LOW;
constexpr uint8_t ACC_OFF = HIGH;

}  // namespace Logic

namespace Timing {

constexpr uint32_t ACC_DEBOUNCE_MS = 100;
constexpr uint32_t DEBUG_PRINT_INTERVAL_MS = 1000;

constexpr uint32_t ACC_ALARM_CONFIRM_MS = 300;
constexpr uint32_t ALARM_INPUT_CLEAR_MS = 2000;

constexpr uint32_t MOTION_SAMPLE_INTERVAL_MS = 20;
constexpr uint16_t MOTION_BASELINE_SAMPLES = 75;
constexpr uint32_t MOTION_CONFIRM_MS = 250;

constexpr uint32_t GPS_DATA_TIMEOUT_MS = 3000;
constexpr uint32_t GPS_FIX_MAX_AGE_MS = 15000;

constexpr uint32_t GSM_BOOT_WAIT_MS = 5000;
constexpr uint32_t GSM_AT_TIMEOUT_MS = 3000;
constexpr uint32_t GSM_SMS_SEND_TIMEOUT_MS = 20000;
constexpr uint32_t GSM_RETRY_INTERVAL_MS = 3000;

/*
 * Lấy một mẫu điện áp mỗi 50 ms.
 * Bộ lọc trung bình dùng 16 mẫu, tương đương khoảng 0,8 giây.
 */
constexpr uint32_t BATTERY_SAMPLE_INTERVAL_MS = 50;
constexpr uint8_t BATTERY_FILTER_SAMPLE_COUNT = 16;

constexpr uint32_t FIND_DURATION_MS = 6000;
constexpr uint32_t FIND_SIREN_PERIOD_MS = 1000;
constexpr uint32_t FIND_SIREN_ON_MS = 120;

constexpr uint32_t TURN_SIGNAL_PERIOD_MS = 500;
constexpr uint32_t TURN_SIGNAL_ON_MS = 250;

constexpr uint32_t ALARM_SIREN_PERIOD_MS = 1500;
constexpr uint32_t ALARM_SIREN_ON_MS = 1000;

}  // namespace Timing

namespace Motion {

constexpr float VIBRATION_THRESHOLD_MS2 = 2.2F;
constexpr float TILT_THRESHOLD_DEG = 12.0F;

}  // namespace Motion

namespace Battery {

/*
 * Code đang giả sử phần cứng cuối cùng dùng:
 * - R18: 68 kΩ từ ắc quy tới GPIO35;
 * - R19: 12 kΩ từ GPIO35 xuống GND.
 *
 * Nếu phần cứng thực tế dùng giá trị khác,
 * phải sửa đúng hai hằng số này.
 */
constexpr float R_TOP_OHM = 68000.0F;
constexpr float R_BOTTOM_OHM = 12000.0F;

/*
 * Hệ số hiệu chỉnh sau khi so sánh với đồng hồ đo.
 * Ví dụ code báo 12,30 V nhưng đồng hồ là 12,50 V:
 * CALIBRATION_FACTOR = 12,50 / 12,30.
 */
constexpr float CALIBRATION_FACTOR = 1.000F;

}  // namespace Battery

constexpr uint32_t DEBUG_BAUD = 115200;
constexpr uint32_t GPS_BAUD = 9600;
constexpr uint32_t GSM_BAUD = 115200;

constexpr uint8_t ESPNOW_CHANNEL = 6;
constexpr uint32_t I2C_FREQUENCY_HZ = 400000;

}  // namespace MainConfig
