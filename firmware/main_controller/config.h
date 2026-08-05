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
}

namespace Logic {
constexpr uint8_t STARTER_LOCK_ACTIVE = HIGH;
constexpr uint8_t STARTER_LOCK_INACTIVE = LOW;
constexpr uint8_t SIREN_ACTIVE = HIGH;
constexpr uint8_t SIREN_INACTIVE = LOW;
constexpr uint8_t TURN_SIGNAL_ACTIVE = HIGH;
constexpr uint8_t TURN_SIGNAL_INACTIVE = LOW;
constexpr uint8_t ACC_ON = LOW;
constexpr uint8_t ACC_OFF = HIGH;
}

namespace Timing {
constexpr uint32_t ACC_DEBOUNCE_MS = 100;
constexpr uint32_t DEBUG_PRINT_INTERVAL_MS = 1000;
}

constexpr uint32_t DEBUG_BAUD = 115200;
constexpr uint8_t ESPNOW_CHANNEL = 6;

}
