#include <Arduino.h>

#include "config.h"
#include "espnow_manager.h"
#include "output_controller.h"
#include "protocol.h"
#include "vehicle_inputs.h"

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
    Serial.begin(MainConfig::DEBUG_BAUD);
    delay(500);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" MOTORCYCLE ANTI-THEFT - MAIN PHASE 2");
    Serial.println("======================================");

    OutputController::begin();
    VehicleInputs::begin();
    EspNowManager::begin();

    Serial.println("Phase 2 initialization completed.");
}

void loop()
{
    VehicleInputs::update();
    EspNowManager::update();

    RemoteCommand command = RemoteCommand::NONE;
    uint32_t packetId = 0;

    if (EspNowManager::getPendingCommand(
            command,
            packetId)) {
        Serial.print("[MAIN] Received command: ");
        Serial.println(commandToText(command));

        /*
         * Mốc 2 chỉ kiểm tra truyền thông.
         * Chưa tác động relay, còi hoặc xi-nhan.
         */
        EspNowManager::sendResponse(
            packetId,
            command,
            CommandResult::SUCCESS,
            SystemState::DISARMED);
    }

    delay(1);
}
