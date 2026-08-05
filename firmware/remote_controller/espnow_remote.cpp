#include "espnow_remote.h"

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "ESP32_NOW.h"

#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

namespace EspNowRemote {
namespace {

portMUX_TYPE responseMux =
    portMUX_INITIALIZER_UNLOCKED;

ControllerResponsePacket receivedResponse{};
volatile bool responsePending = false;

bool managerReady = false;
bool waitingForAck = false;
bool resultAvailable = false;

RemoteCommandPacket activePacket{};

uint8_t sendAttempt = 0;
uint32_t lastSendAt = 0;

RemoteCommand completedCommand =
    RemoteCommand::NONE;

CommandResult completedResult =
    CommandResult::INVALID;

SystemState completedState =
    SystemState::DISARMED;

/*
 * Packet ID được lưu trong NVS để sau khi remote mất nguồn
 * vẫn tiếp tục tăng, thay vì quay lại từ 1.
 */
Preferences packetPreferences;

constexpr char PACKET_NAMESPACE[] =
    "remote";

constexpr char PACKET_KEY[] =
    "packet";

bool packetStorageReady = false;
uint32_t packetCounter = 0;

bool isZeroMac(const uint8_t *mac)
{
    for (uint8_t index = 0; index < 6; ++index) {
        if (mac[index] != 0) {
            return false;
        }
    }

    return true;
}

uint32_t createNextPacketId()
{
    ++packetCounter;

    if (packetCounter == 0) {
        packetCounter = 1;
    }

    if (packetStorageReady) {
        const size_t bytesWritten =
            packetPreferences.putUInt(
                PACKET_KEY,
                packetCounter);

        if (bytesWritten == 0) {
            Serial.println(
                "[ESP-NOW REMOTE] Cannot persist packet ID.");
        }
    }

    return packetCounter;
}

class MainPeer : public ESP_NOW_Peer {
public:
    explicit MainPeer(
        const uint8_t *macAddress)
        : ESP_NOW_Peer(
              macAddress,
              RemoteConfig::ESPNOW_CHANNEL,
              WIFI_IF_STA,
              nullptr)
    {
    }

    bool registerPeer()
    {
        return add();
    }

    bool sendPacket(
        const RemoteCommandPacket &packet)
    {
        return send(
                   reinterpret_cast<const uint8_t *>(
                       &packet),
                   sizeof(packet)) ==
               sizeof(packet);
    }

    void onReceive(
        const uint8_t *data,
        size_t length,
        bool broadcast) override
    {
        (void)broadcast;

        if (length !=
            sizeof(ControllerResponsePacket)) {
            return;
        }

        portENTER_CRITICAL(&responseMux);

        memcpy(
            &receivedResponse,
            data,
            sizeof(ControllerResponsePacket));

        responsePending = true;

        portEXIT_CRITICAL(&responseMux);
    }

    void onSent(bool success) override
    {
        Serial.print(
            "[ESP-NOW REMOTE] Packet transmit: ");

        Serial.println(
            success ? "SUCCESS" : "FAILED");
    }
};

MainPeer mainPeer(
    MAIN_CONTROLLER_MAC_ADDRESS);

bool copyPendingResponse(
    ControllerResponsePacket &response)
{
    if (!responsePending) {
        return false;
    }

    portENTER_CRITICAL(&responseMux);

    memcpy(
        &response,
        &receivedResponse,
        sizeof(response));

    responsePending = false;

    portEXIT_CRITICAL(&responseMux);

    return true;
}

void finishCommand(
    CommandResult result,
    SystemState state)
{
    completedCommand =
        static_cast<RemoteCommand>(
            activePacket.command);

    completedResult = result;
    completedState = state;

    resultAvailable = true;
    waitingForAck = false;
}

bool transmitActivePacket()
{
    ++sendAttempt;
    lastSendAt = millis();

    Serial.print(
        "[ESP-NOW REMOTE] Send attempt ");
    Serial.print(sendAttempt);

    Serial.print(", packetId=");
    Serial.println(activePacket.packetId);

    return mainPeer.sendPacket(
        activePacket);
}

void initializePacketStorage()
{
    packetStorageReady =
        packetPreferences.begin(
            PACKET_NAMESPACE,
            false);

    if (!packetStorageReady) {
        packetCounter = 0;

        Serial.println(
            "[ESP-NOW REMOTE] Packet NVS unavailable.");

        return;
    }

    packetCounter =
        packetPreferences.getUInt(
            PACKET_KEY,
            0);

    Serial.print(
        "[ESP-NOW REMOTE] Restored packet ID: ");
    Serial.println(packetCounter);
}

}  // namespace

bool begin()
{
    initializePacketStorage();

    WiFi.mode(WIFI_STA);
    WiFi.setChannel(
        RemoteConfig::ESPNOW_CHANNEL);

    while (!WiFi.STA.started()) {
        delay(10);
    }

    Serial.print(
        "[ESP-NOW REMOTE] Local MAC: ");
    Serial.println(WiFi.macAddress());

    if (!ESP_NOW.begin()) {
        Serial.println(
            "[ESP-NOW REMOTE] Initialization failed.");

        managerReady = false;
        return false;
    }

    if (isZeroMac(
            MAIN_CONTROLLER_MAC_ADDRESS)) {
        Serial.println(
            "[ESP-NOW REMOTE] Main MAC is not configured.");

        managerReady = false;
        return false;
    }

    if (!mainPeer.registerPeer()) {
        Serial.println(
            "[ESP-NOW REMOTE] Cannot register main peer.");

        managerReady = false;
        return false;
    }

    managerReady = true;

    Serial.print(
        "[ESP-NOW REMOTE] Channel: ");
    Serial.println(
        RemoteConfig::ESPNOW_CHANNEL);

    Serial.println(
        "[ESP-NOW REMOTE] Ready.");

    return true;
}

void update()
{
    if (!waitingForAck) {
        return;
    }

    ControllerResponsePacket response{};

    if (copyPendingResponse(response)) {
        if (response.packetId !=
                activePacket.packetId ||
            response.command !=
                activePacket.command) {
            Serial.println(
                "[ESP-NOW REMOTE] Ignore unmatched ACK.");
        } else {
            finishCommand(
                static_cast<CommandResult>(
                    response.result),
                static_cast<SystemState>(
                    response.systemState));

            return;
        }
    }

    if (millis() - lastSendAt <
        RemoteConfig::Timing::
            ACK_TIMEOUT_MS) {
        return;
    }

    if (sendAttempt <
        RemoteConfig::Timing::
            MAX_SEND_ATTEMPTS) {
        /*
         * Gửi lại đúng activePacket:
         * packetId và authenticationCode không đổi.
         */
        transmitActivePacket();
        return;
    }

    Serial.println(
        "[ESP-NOW REMOTE] ACK timeout.");

    finishCommand(
        CommandResult::BUSY,
        SystemState::DISARMED);
}

bool isReady()
{
    return managerReady;
}

bool isBusy()
{
    return waitingForAck;
}

bool requestCommand(
    RemoteCommand command)
{
    if (!managerReady ||
        waitingForAck ||
        command == RemoteCommand::NONE) {
        return false;
    }

    activePacket.packetId =
        createNextPacketId();

    activePacket.command =
        static_cast<uint8_t>(command);

    activePacket.authenticationCode =
        makeAuthenticationCode(
            ESPNOW_SHARED_SECRET,
            activePacket.packetId,
            activePacket.command);

    sendAttempt = 0;
    responsePending = false;
    resultAvailable = false;
    waitingForAck = true;

    if (!transmitActivePacket()) {
        Serial.println(
            "[ESP-NOW REMOTE] Initial send failed.");
    }

    return true;
}

bool getLastResult(
    RemoteCommand &command,
    CommandResult &result,
    SystemState &systemState)
{
    if (!resultAvailable) {
        return false;
    }

    command = completedCommand;
    result = completedResult;
    systemState = completedState;

    resultAvailable = false;

    return true;
}

}  // namespace EspNowRemote
