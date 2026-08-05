#include <Arduino.h>

#include "config.h"
#include "espnow_manager.h"
#include "gps_manager.h"
#include "motion_sensor.h"
#include "output_controller.h"
#include "protocol.h"
#include "state_machine.h"
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

void setup()
{
    Serial.begin(MainConfig::DEBUG_BAUD);
    delay(500);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" MOTORCYCLE ANTI-THEFT - MAIN PHASE 6");
    Serial.println("======================================");

    OutputController::begin();
    VehicleInputs::begin();
    MotionSensor::begin();
    GpsManager::begin();
    StateMachine::begin();
    EspNowManager::begin();

    Serial.println("Phase 6 initialization completed.");
}

void loop()
{
    VehicleInputs::update();
    EspNowManager::update();
    GpsManager::update();

    MotionSensor::setMonitoringEnabled(
        StateMachine::isAntiTheftArmed());
    MotionSensor::update();

    AlarmInputs alarmInputs;
    alarmInputs.accOn =
        VehicleInputs::isAccOn();
    alarmInputs.motionDetected =
        MotionSensor::isMotionDetected();

    StateMachine::update(alarmInputs);

    RemoteCommand command = RemoteCommand::NONE;
    uint32_t packetId = 0;

    if (EspNowManager::getPendingCommand(
            command,
            packetId)) {
        Serial.print("[MAIN] Received command: ");
        Serial.println(commandToText(command));

        const CommandResult result =
            StateMachine::handleCommand(command);

        EspNowManager::sendResponse(
            packetId,
            command,
            result,
            StateMachine::getState());
    }

    static uint32_t lastPrintAt = 0;
    const uint32_t now = millis();

    if (now - lastPrintAt >=
        MainConfig::Timing::DEBUG_PRINT_INTERVAL_MS) {
        lastPrintAt = now;

        Serial.print("[STATUS] State=");
        Serial.print(StateMachine::getStateText());

        Serial.print(" | ACC=");
        Serial.print(
            VehicleInputs::isAccOn()
                ? "ON"
                : "OFF");

        Serial.print(" | MPU=");
        Serial.print(
            MotionSensor::isAvailable()
                ? "OK"
                : "FAIL");

        Serial.print(" | Motion=");
        Serial.print(
            MotionSensor::isMotionDetected()
                ? "YES"
                : "NO");

        Serial.print(" | GPS data=");
        Serial.print(
            GpsManager::isReceivingData()
                ? "YES"
                : "NO");

        Serial.print(" | GPS fix=");
        Serial.print(
            GpsManager::hasValidFix()
                ? "YES"
                : "NO");

        Serial.print(" | Satellites=");
        Serial.print(
            GpsManager::getSatelliteCount());

        Serial.print(" | GPS chars=");
        Serial.print(
            GpsManager::getCharactersProcessed());

        if (GpsManager::hasValidFix()) {
            Serial.print(" | Lat=");
            Serial.print(
                GpsManager::getLatitude(),
                6);

            Serial.print(" | Lng=");
            Serial.print(
                GpsManager::getLongitude(),
                6);
        }

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
