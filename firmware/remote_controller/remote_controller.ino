#include <Arduino.h>

#include "config.h"
#include "espnow_remote.h"
#include "protocol.h"
#include "remote_io.h"

const char *commandToText(RemoteCommand command)
{
    switch (command) {
        case RemoteCommand::ARM:
            return "ARM";
        case RemoteCommand::DISARM:
            return "DISARM";
        case RemoteCommand::FIND:
            return "FIND";
        case RemoteCommand::SILENCE:
            return "SILENCE";
        default:
            return "NONE";
    }
}

void setup()
{
    Serial.begin(RemoteConfig::DEBUG_BAUD);
    delay(500);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" MOTORCYCLE ANTI-THEFT - REMOTE PHASE 2");
    Serial.println("======================================");

    RemoteIo::begin();
    EspNowRemote::begin();

    Serial.println("Phase 2 initialization completed.");
}

void loop()
{
    RemoteIo::update();
    EspNowRemote::update();

    RemoteCommand pressedCommand = RemoteCommand::NONE;

    if (RemoteIo::getPendingCommand(pressedCommand)) {
        Serial.print("[REMOTE] Button command: ");
        Serial.println(commandToText(pressedCommand));

        if (!EspNowRemote::requestCommand(
                pressedCommand)) {
            Serial.println(
                "[REMOTE] Cannot start transmission.");
            RemoteIo::showCommunicationFailure();
        }
    }

    RemoteCommand completedCommand =
        RemoteCommand::NONE;
    CommandResult result =
        CommandResult::INVALID;
    SystemState systemState =
        SystemState::DISARMED;

    if (EspNowRemote::getLastResult(
            completedCommand,
            result,
            systemState)) {
        Serial.print("[REMOTE] ACK for ");
        Serial.print(commandToText(completedCommand));
        Serial.print(" | result=");
        Serial.print(static_cast<uint8_t>(result));
        Serial.print(" | state=");
        Serial.println(static_cast<uint8_t>(systemState));

        if (result == CommandResult::SUCCESS) {
            RemoteIo::showCommandSuccess(
                completedCommand);
        } else {
            RemoteIo::showCommunicationFailure();
        }
    }

    delay(1);
}
