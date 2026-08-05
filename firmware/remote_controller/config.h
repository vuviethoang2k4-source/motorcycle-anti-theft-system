#pragma once
#include <Arduino.h>

namespace RemoteConfig {

namespace Pin {
constexpr uint8_t BUTTON_ARM = 3;
constexpr uint8_t BUTTON_DISARM = 4;
constexpr uint8_t BUTTON_FIND = 5;
constexpr uint8_t BUTTON_SILENCE = 6;
constexpr uint8_t LED_ARM = 7;
constexpr uint8_t LED_DISARM = 8;
constexpr uint8_t LED_FIND = 10;
constexpr uint8_t LED_SILENCE = 20;
}

namespace Logic {
constexpr uint8_t BUTTON_PRESSED = LOW;
constexpr uint8_t BUTTON_RELEASED = HIGH;
constexpr uint8_t LED_ON = HIGH;
constexpr uint8_t LED_OFF = LOW;
}

namespace Timing {
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;
constexpr uint32_t LED_FEEDBACK_MS = 200;
constexpr uint32_t ACK_TIMEOUT_MS = 800;
constexpr uint8_t MAX_SEND_ATTEMPTS = 3;
}

constexpr uint32_t DEBUG_BAUD = 115200;
constexpr uint8_t ESPNOW_CHANNEL = 6;

}
