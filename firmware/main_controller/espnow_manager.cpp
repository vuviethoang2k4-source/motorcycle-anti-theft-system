#include "espnow_manager.h"

#include <Arduino.h>
#include <WiFi.h>
#include "ESP32_NOW.h"

#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

namespace EspNowManager {
namespace {

portMUX_TYPE receiveMux =
    portMUX_INITIALIZER_UNLOCKED;

RemoteCommandPacket receivedPacket{};
volatile bool packetPending = false;

bool managerReady = false;

/*
 * Bộ nhớ ACK gần nhất:
 * - remote gửi lại cùng packetId khi chưa nhận được ACK;
 * - mạch trung tâm gửi lại response này;
 * - không đưa lệnh lặp ra main_controller.ino.
 */
bool cachedResponseValid = false;
ControllerResponsePacket cachedResponse{};

bool isZeroMac(const uint8_t *mac)
{
    for (uint8_t index = 0; index < 6; ++index) {
        if (mac[index] != 0) {
            return false;
        }
    }

    return true;
}

class RemotePeer : public ESP_NOW_Peer {
public:
    explicit RemotePeer(const uint8_t *macAddress)
        : ESP_NOW_Peer(
              macAddress,
              MainConfig::ESPNOW_CHANNEL,
              WIFI_IF_STA,
              nullptr)
    {
    }

    bool registerPeer()
    {
        return add();
    }

    bool sendPacket(
        const ControllerResponsePacket &response)
    {
        return send(
                   reinterpret_cast<const uint8_t *>(
                       &response),
                   sizeof(response)) ==
               sizeof(response);
    }

    void onReceive(
        const uint8_t *data,
        size_t length,
        bool broadcast) override
    {
        (void)broadcast;

        if (length !=
            sizeof(RemoteCommandPacket)) {
            return;
        }

        portENTER_CRITICAL(&receiveMux);

        memcpy(
            &receivedPacket,
            data,
            sizeof(RemoteCommandPacket));

        packetPending = true;

        portEXIT_CRITICAL(&receiveMux);
    }

    void onSent(bool success) override
    {
        Serial.print(
            "[ESP-NOW MAIN] ACK transmit: ");

        Serial.println(
            success ? "SUCCESS" : "FAILED");
    }
};

RemotePeer remotePeer(REMOTE_MAC_ADDRESS);

bool copyPendingPacket(
    RemoteCommandPacket &packet)
{
    if (!packetPending) {
        return false;
    }

    portENTER_CRITICAL(&receiveMux);

    memcpy(
        &packet,
        &receivedPacket,
        sizeof(packet));

    packetPending = false;

    portEXIT_CRITICAL(&receiveMux);

    return true;
}

bool isDuplicateOfCachedResponse(
    const RemoteCommandPacket &packet)
{
    return cachedResponseValid &&
           cachedResponse.packetId ==
               packet.packetId &&
           cachedResponse.command ==
               packet.command;
}

bool resendCachedResponse()
{
    if (!managerReady ||
        !cachedResponseValid) {
        return false;
    }

    Serial.print(
        "[ESP-NOW MAIN] Duplicate packet ");
    Serial.print(cachedResponse.packetId);
    Serial.println(
        ", resend cached ACK without executing command.");

    return remotePeer.sendPacket(
        cachedResponse);
}

}  // namespace

bool begin()
{
    WiFi.mode(WIFI_STA);
    WiFi.setChannel(
        MainConfig::ESPNOW_CHANNEL);

    while (!WiFi.STA.started()) {
        delay(10);
    }

    Serial.print(
        "[ESP-NOW MAIN] Local MAC: ");
    Serial.println(WiFi.macAddress());

    if (!ESP_NOW.begin()) {
        Serial.println(
            "[ESP-NOW MAIN] Initialization failed.");

        managerReady = false;
        return false;
    }

    if (isZeroMac(
            REMOTE_MAC_ADDRESS)) {
        Serial.println(
            "[ESP-NOW MAIN] Remote MAC is not configured.");

        managerReady = false;
        return false;
    }

    if (!remotePeer.registerPeer()) {
        Serial.println(
            "[ESP-NOW MAIN] Cannot register remote peer.");

        managerReady = false;
        return false;
    }

    managerReady = true;
    cachedResponseValid = false;

    Serial.print(
        "[ESP-NOW MAIN] Channel: ");
    Serial.println(
        MainConfig::ESPNOW_CHANNEL);

    Serial.println(
        "[ESP-NOW MAIN] Ready.");

    return true;
}

void update()
{
    /*
     * Callback chỉ chép dữ liệu vào bộ đệm.
     * Việc xác thực và xử lý diễn ra trong loop().
     */
}

bool isReady()
{
    return managerReady;
}

bool getPendingCommand(
    RemoteCommand &command,
    uint32_t &packetId)
{
    RemoteCommandPacket packet{};

    if (!copyPendingPacket(packet)) {
        return false;
    }

    if (!isSupportedCommand(
            packet.command)) {
        sendResponse(
            packet.packetId,
            RemoteCommand::NONE,
            CommandResult::INVALID,
            SystemState::DISARMED);

        Serial.println(
            "[ESP-NOW MAIN] Invalid command.");

        return false;
    }

    const uint32_t expectedCode =
        makeAuthenticationCode(
            ESPNOW_SHARED_SECRET,
            packet.packetId,
            packet.command);

    if (packet.authenticationCode !=
        expectedCode) {
        sendResponse(
            packet.packetId,
            static_cast<RemoteCommand>(
                packet.command),
            CommandResult::
                AUTHENTICATION_ERROR,
            SystemState::DISARMED);

        Serial.println(
            "[ESP-NOW MAIN] Authentication error.");

        return false;
    }

    /*
     * Chỉ kiểm tra trùng sau khi xác thực thành công.
     */
    if (isDuplicateOfCachedResponse(
            packet)) {
        resendCachedResponse();
        return false;
    }

    command =
        static_cast<RemoteCommand>(
            packet.command);

    packetId = packet.packetId;

    return true;
}

bool sendResponse(
    uint32_t packetId,
    RemoteCommand command,
    CommandResult result,
    SystemState currentState)
{
    if (!managerReady) {
        return false;
    }

    ControllerResponsePacket response{};

    response.packetId = packetId;
    response.command =
        static_cast<uint8_t>(command);
    response.result =
        static_cast<uint8_t>(result);
    response.systemState =
        static_cast<uint8_t>(
            currentState);

    /*
     * Lưu trước khi gửi.
     * Nếu ACK bị mất trên đường truyền, remote gửi lại
     * và mạch trung tâm vẫn có response để trả lại.
     */
    cachedResponse = response;
    cachedResponseValid = true;

    return remotePeer.sendPacket(response);
}

}  // namespace EspNowManager
