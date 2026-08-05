#pragma once

#include <Arduino.h>

/*
 * Giao thức chung giữa remote và mạch trung tâm.
 * Hai file protocol.h ở hai phía phải giống hoàn toàn.
 */
enum class RemoteCommand : uint8_t {
    NONE = 0,
    ARM = 1,
    DISARM = 2,
    FIND = 3,
    SILENCE = 4
};

enum class SystemState : uint8_t {
    DISARMED = 0,
    ARMED = 1,
    ALARM = 2,
    SILENCED = 3,
    FINDING = 4
};

enum class CommandResult : uint8_t {
    INVALID = 0,
    SUCCESS = 1,
    AUTHENTICATION_ERROR = 2,
    BUSY = 3
};

struct __attribute__((packed)) RemoteCommandPacket {
    uint32_t packetId;
    uint8_t command;
    uint32_t authenticationCode;
};

struct __attribute__((packed)) ControllerResponsePacket {
    uint32_t packetId;
    uint8_t command;
    uint8_t result;
    uint8_t systemState;
};

inline bool isSupportedCommand(uint8_t command)
{
    return command >= static_cast<uint8_t>(RemoteCommand::ARM) &&
           command <= static_cast<uint8_t>(RemoteCommand::SILENCE);
}

/*
 * Mã xác thực cơ bản dùng cho đồ án.
 * Hai phía phải sử dụng cùng ESPNOW_SHARED_SECRET.
 */
inline uint32_t makeAuthenticationCode(
    uint32_t sharedSecret,
    uint32_t packetId,
    uint8_t command)
{
    uint32_t value = sharedSecret;

    value ^= packetId + 0x9E3779B9UL;
    value ^= static_cast<uint32_t>(command) * 0x85EBCA6BUL;

    value ^= value >> 16;
    value *= 0x7FEB352DUL;
    value ^= value >> 15;
    value *= 0x846CA68BUL;
    value ^= value >> 16;

    return value;
}
