#include "state_machine.h"

#include <Arduino.h>

#include "config.h"
#include "output_controller.h"

namespace StateMachine {
namespace {

SystemState currentState = SystemState::DISARMED;
SystemState stateBeforeFinding = SystemState::DISARMED;

uint32_t findingStartedAt = 0;
uint32_t accActiveSince = 0;
uint32_t alarmInputClearSince = 0;

const char *stateToText(SystemState state)
{
    switch (state) {
        case SystemState::DISARMED:
            return "DISARMED";

        case SystemState::ARMED:
            return "ARMED";

        case SystemState::ALARM:
            return "ALARM";

        case SystemState::SILENCED:
            return "SILENCED";

        case SystemState::FINDING:
            return "FINDING";

        default:
            return "UNKNOWN";
    }
}

bool requiresStarterLock(SystemState state)
{
    switch (state) {
        case SystemState::ARMED:
        case SystemState::ALARM:
        case SystemState::SILENCED:
            return true;

        case SystemState::FINDING:
            return requiresStarterLock(stateBeforeFinding);

        case SystemState::DISARMED:
        default:
            return false;
    }
}

bool waitingForNewAlarm()
{
    if (currentState == SystemState::ARMED) {
        return true;
    }

    return currentState == SystemState::FINDING &&
           stateBeforeFinding == SystemState::ARMED;
}

bool alarmIsSilenced()
{
    if (currentState == SystemState::SILENCED) {
        return true;
    }

    return currentState == SystemState::FINDING &&
           stateBeforeFinding == SystemState::SILENCED;
}

void applyStarterLock()
{
    OutputController::setStarterLocked(
        requiresStarterLock(currentState));
}

void resetAlarmTimers()
{
    accActiveSince = 0;
    alarmInputClearSince = 0;
}

void enterDisarmed()
{
    currentState = SystemState::DISARMED;
    stateBeforeFinding = SystemState::DISARMED;

    resetAlarmTimers();

    OutputController::setStarterLocked(false);
    OutputController::stopWarningOutputs();

    Serial.println("[STATE] DISARMED");
}

void enterArmed()
{
    currentState = SystemState::ARMED;
    stateBeforeFinding = SystemState::ARMED;

    resetAlarmTimers();

    OutputController::setStarterLocked(true);
    OutputController::stopWarningOutputs();

    Serial.println("[STATE] ARMED");
}

void enterAlarm(const char *reason)
{
    currentState = SystemState::ALARM;
    stateBeforeFinding = SystemState::ALARM;

    resetAlarmTimers();

    OutputController::setStarterLocked(true);

    Serial.print("[ALARM] Triggered by ");
    Serial.println(reason);
}

void startFinding()
{
    if (currentState != SystemState::FINDING) {
        stateBeforeFinding = currentState;
    }

    currentState = SystemState::FINDING;
    findingStartedAt = millis();

    applyStarterLock();

    Serial.print("[STATE] FINDING, return to ");
    Serial.println(stateToText(stateBeforeFinding));
}

void silenceWarnings()
{
    OutputController::stopWarningOutputs();

    if (currentState == SystemState::FINDING) {
        currentState = stateBeforeFinding;
    }

    if (currentState == SystemState::ALARM) {
        currentState = SystemState::SILENCED;
        alarmInputClearSince = 0;
    }

    applyStarterLock();

    Serial.print("[STATE] ");
    Serial.println(stateToText(currentState));
}

void updateFinding()
{
    const uint32_t elapsed =
        millis() - findingStartedAt;

    if (elapsed >=
        MainConfig::Timing::FIND_DURATION_MS) {
        OutputController::stopWarningOutputs();

        currentState = stateBeforeFinding;
        applyStarterLock();

        Serial.print(
            "[STATE] FIND completed, return to ");
        Serial.println(stateToText(currentState));
        return;
    }

    const bool sirenOn =
        elapsed %
            MainConfig::Timing::FIND_SIREN_PERIOD_MS <
        MainConfig::Timing::FIND_SIREN_ON_MS;

    const bool turnOn =
        elapsed %
            MainConfig::Timing::TURN_SIGNAL_PERIOD_MS <
        MainConfig::Timing::TURN_SIGNAL_ON_MS;

    OutputController::setSiren(sirenOn);
    OutputController::setTurnSignal(turnOn);
}

void updateAlarmPattern()
{
    const uint32_t elapsed = millis();

    const bool sirenOn =
        elapsed %
            MainConfig::Timing::ALARM_SIREN_PERIOD_MS <
        MainConfig::Timing::ALARM_SIREN_ON_MS;

    const bool turnOn =
        elapsed %
            MainConfig::Timing::TURN_SIGNAL_PERIOD_MS <
        MainConfig::Timing::TURN_SIGNAL_ON_MS;

    OutputController::setSiren(sirenOn);
    OutputController::setTurnSignal(turnOn);
}

void processSilencedState(
    const AlarmInputs &inputs)
{
    const bool anyInputActive =
        inputs.accOn ||
        inputs.motionDetected;

    if (anyInputActive) {
        alarmInputClearSince = 0;
        return;
    }

    if (alarmInputClearSince == 0) {
        alarmInputClearSince = millis();
        return;
    }

    if (millis() - alarmInputClearSince <
        MainConfig::Timing::ALARM_INPUT_CLEAR_MS) {
        return;
    }

    if (currentState == SystemState::SILENCED) {
        currentState = SystemState::ARMED;
    } else {
        stateBeforeFinding = SystemState::ARMED;
    }

    alarmInputClearSince = 0;

    Serial.println(
        "[STATE] Alarm inputs cleared, return to ARMED.");
}

void processAccAlarm(bool accOn)
{
    if (!accOn) {
        accActiveSince = 0;
        return;
    }

    if (accActiveSince == 0) {
        accActiveSince = millis();
        return;
    }

    if (millis() - accActiveSince >=
        MainConfig::Timing::ACC_ALARM_CONFIRM_MS) {
        enterAlarm("ACC");
    }
}

void processAlarmInputs(
    const AlarmInputs &inputs)
{
    if (currentState == SystemState::DISARMED) {
        resetAlarmTimers();
        return;
    }

    if (currentState == SystemState::ALARM) {
        return;
    }

    if (alarmIsSilenced()) {
        processSilencedState(inputs);
        return;
    }

    if (!waitingForNewAlarm()) {
        accActiveSince = 0;
        return;
    }

    /*
     * MotionSensor đã xác nhận rung/nghiêng đủ thời gian,
     * nên có thể kích hoạt báo động ngay tại đây.
     */
    if (inputs.motionDetected) {
        enterAlarm("VIBRATION OR TILT");
        return;
    }

    processAccAlarm(inputs.accOn);
}

}  // namespace

void begin()
{
    enterDisarmed();
}

void update(const AlarmInputs &inputs)
{
    processAlarmInputs(inputs);
    applyStarterLock();

    switch (currentState) {
        case SystemState::FINDING:
            updateFinding();
            break;

        case SystemState::ALARM:
            updateAlarmPattern();
            break;

        default:
            OutputController::stopWarningOutputs();
            break;
    }
}

CommandResult handleCommand(RemoteCommand command)
{
    switch (command) {
        case RemoteCommand::ARM:
            enterArmed();
            return CommandResult::SUCCESS;

        case RemoteCommand::DISARM:
            enterDisarmed();
            return CommandResult::SUCCESS;

        case RemoteCommand::FIND:
            startFinding();
            return CommandResult::SUCCESS;

        case RemoteCommand::SILENCE:
            silenceWarnings();
            return CommandResult::SUCCESS;

        default:
            return CommandResult::INVALID;
    }
}

SystemState getState()
{
    return currentState;
}

const char *getStateText()
{
    return stateToText(currentState);
}

bool isAntiTheftArmed()
{
    return requiresStarterLock(currentState);
}

}  // namespace StateMachine
