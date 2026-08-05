#include <Arduino.h>

#include "config.h"
#include "espnow_manager.h"
#include "gps_manager.h"
#include "motion_sensor.h"
#include "output_controller.h"
#include "protocol.h"
#include "sms_manager.h"
#include "state_machine.h"
#include "storage_manager.h"
#include "system_types.h"
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

RemoteCommand smsRequestToRemoteCommand(
    SmsRequestType request)
{
    switch (request) {
        case SmsRequestType::ARM:
            return RemoteCommand::ARM;

        case SmsRequestType::DISARM:
            return RemoteCommand::DISARM;

        case SmsRequestType::FIND:
            return RemoteCommand::FIND;

        case SmsRequestType::SILENCE:
            return RemoteCommand::SILENCE;

        default:
            return RemoteCommand::NONE;
    }
}

/*
 * Mọi nguồn điều khiển đều đi qua cùng một hàm:
 * - remote ESP-NOW;
 * - SMS.
 *
 * Nhờ đó ARM/DISARM luôn được lưu nhất quán vào NVS.
 */
CommandResult executeSystemCommand(
    RemoteCommand command)
{
    const CommandResult result =
        StateMachine::handleCommand(command);

    if (result != CommandResult::SUCCESS) {
        return result;
    }

    if (command == RemoteCommand::ARM) {
        StorageManager::saveAntiTheftArmed(true);
    } else if (
        command == RemoteCommand::DISARM) {
        StorageManager::saveAntiTheftArmed(false);
    }

    return result;
}

String buildStatusMessage()
{
    String message = "STATE: ";
    message += StateMachine::getStateText();

    message += "\nACC: ";
    message += VehicleInputs::isAccOn()
                   ? "ON"
                   : "OFF";

    message += "\nMOTION: ";
    message += MotionSensor::isMotionDetected()
                   ? "YES"
                   : "NO";

    message += "\nGPS: ";
    message += GpsManager::hasValidFix()
                   ? "FIX"
                   : "NO FIX";

    message += "\nSAT: ";
    message += String(
        GpsManager::getSatelliteCount());

    return message;
}

String buildCommandReply(
    RemoteCommand command,
    CommandResult result)
{
    String reply = "COMMAND: ";
    reply += commandToText(command);

    reply += "\nRESULT: ";
    reply += result == CommandResult::SUCCESS
                 ? "SUCCESS"
                 : "FAILED";

    reply += "\nSTATE: ";
    reply += StateMachine::getStateText();

    return reply;
}

void processSmsRequests()
{
    SmsRequestType request =
        SmsRequestType::NONE;

    if (!SmsManager::getPendingRequest(request)) {
        return;
    }

    if (request == SmsRequestType::STATUS) {
        SmsManager::queueMessage(
            buildStatusMessage());
        return;
    }

    if (request == SmsRequestType::LOCATION) {
        SmsManager::queueMessage(
            GpsManager::getGoogleMapsUrl());
        return;
    }

    const RemoteCommand command =
        smsRequestToRemoteCommand(request);

    if (command == RemoteCommand::NONE) {
        SmsManager::queueMessage(
            "INVALID COMMAND");
        return;
    }

    const CommandResult result =
        executeSystemCommand(command);

    SmsManager::queueMessage(
        buildCommandReply(command, result));
}

void queueAlarmSmsOnStateTransition()
{
    static SystemState previousState =
        SystemState::DISARMED;

    const SystemState currentState =
        StateMachine::getState();

    if (currentState == SystemState::ALARM &&
        previousState != SystemState::ALARM) {
        String message = "THEFT ALERT";

        message += "\nREASON: ";
        message +=
            StateMachine::getAlarmReasonText();

        message += "\n";
        message += GpsManager::getGoogleMapsUrl();

        SmsManager::queueMessage(message);
    }

    previousState = currentState;
}

void restorePersistedAntiTheftState()
{
    if (!StorageManager::isReady()) {
        Serial.println(
            "[STORAGE] Start in DISARMED because NVS is unavailable.");
        return;
    }

    if (!StorageManager::loadAntiTheftArmed()) {
        Serial.println(
            "[STORAGE] Restored state: DISARMED");
        return;
    }

    /*
     * Chỉ khôi phục ARMED, không khôi phục ALARM/FIND/SILENCE.
     */
    StateMachine::handleCommand(
        RemoteCommand::ARM);

    Serial.println(
        "[STORAGE] Restored state: ARMED");
}

void setup()
{
    Serial.begin(MainConfig::DEBUG_BAUD);
    delay(500);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" MOTORCYCLE ANTI-THEFT - MAIN PHASE 8");
    Serial.println("======================================");

    OutputController::begin();
    VehicleInputs::begin();
    MotionSensor::begin();
    GpsManager::begin();
    SmsManager::begin();

    StorageManager::begin();
    StateMachine::begin();
    restorePersistedAntiTheftState();

    EspNowManager::begin();

    Serial.println("Phase 8 initialization completed.");
}

void loop()
{
    VehicleInputs::update();
    GpsManager::update();
    SmsManager::update();
    EspNowManager::update();

    MotionSensor::setMonitoringEnabled(
        StateMachine::isAntiTheftArmed());
    MotionSensor::update();

    AlarmInputs alarmInputs;
    alarmInputs.accOn =
        VehicleInputs::isAccOn();
    alarmInputs.motionDetected =
        MotionSensor::isMotionDetected();

    StateMachine::update(alarmInputs);

    RemoteCommand espNowCommand =
        RemoteCommand::NONE;
    uint32_t packetId = 0;

    if (EspNowManager::getPendingCommand(
            espNowCommand,
            packetId)) {
        const CommandResult result =
            executeSystemCommand(
                espNowCommand);

        EspNowManager::sendResponse(
            packetId,
            espNowCommand,
            result,
            StateMachine::getState());
    }

    processSmsRequests();
    queueAlarmSmsOnStateTransition();

    static uint32_t lastPrintAt = 0;
    const uint32_t now = millis();

    if (now - lastPrintAt >=
        MainConfig::Timing::DEBUG_PRINT_INTERVAL_MS) {
        lastPrintAt = now;

        Serial.print("[STATUS] State=");
        Serial.print(StateMachine::getStateText());

        Serial.print(" | NVS=");
        Serial.print(
            StorageManager::isReady()
                ? "READY"
                : "FAIL");

        Serial.print(" | ACC=");
        Serial.print(
            VehicleInputs::isAccOn()
                ? "ON"
                : "OFF");

        Serial.print(" | Motion=");
        Serial.print(
            MotionSensor::isMotionDetected()
                ? "YES"
                : "NO");

        Serial.print(" | GPS=");
        Serial.print(
            GpsManager::hasValidFix()
                ? "FIX"
                : "NO FIX");

        Serial.print(" | GSM=");
        Serial.print(
            SmsManager::isReady()
                ? "READY"
                : "NOT READY");

        Serial.print(" | SMS RX=");
        Serial.print(
            SmsManager::getReceivedSmsCount());

        Serial.print(" | SMS TX=");
        Serial.print(
            SmsManager::getSentSmsCount());

        Serial.print(" | SMS FAIL=");
        Serial.print(
            SmsManager::getFailedSmsCount());

        Serial.print(" | StarterLock=");
        Serial.print(
            OutputController::isStarterLocked()
                ? "ON"
                : "OFF");

        Serial.print(" | Siren=");
        Serial.print(
            OutputController::isSirenEnabled()
                ? "ON"
                : "OFF");

        Serial.print(" | Turn=");
        Serial.println(
            OutputController::isTurnSignalEnabled()
                ? "ON"
                : "OFF");
    }

    delay(1);
}
