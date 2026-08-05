#include "state_machine.h"

#include <Arduino.h>

#include "config.h"
#include "output_controller.h"

namespace StateMachine {
namespace {

SystemState currentState = SystemState::DISARMED;
SystemState stateBeforeFinding = SystemState::DISARMED;
uint32_t findingStartedAt = 0;

const char *stateToText(SystemState state)
{
    switch (state) {
        case SystemState::DISARMED: return "DISARMED";
        case SystemState::ARMED: return "ARMED";
        case SystemState::ALARM: return "ALARM";
        case SystemState::SILENCED: return "SILENCED";
        case SystemState::FINDING: return "FINDING";
        default: return "UNKNOWN";
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

void applyStarterLock()
{
    OutputController::setStarterLocked(
        requiresStarterLock(currentState));
}

void enterDisarmed()
{
    currentState = SystemState::DISARMED;
    stateBeforeFinding = SystemState::DISARMED;
    OutputController::setStarterLocked(false);
    OutputController::stopWarningOutputs();
    Serial.println("[STATE] DISARMED");
}

void enterArmed()
{
    currentState = SystemState::ARMED;
    stateBeforeFinding = SystemState::ARMED;
    OutputController::setStarterLocked(true);
    OutputController::stopWarningOutputs();
    Serial.println("[STATE] ARMED");
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
    }

    applyStarterLock();

    Serial.print("[STATE] ");
    Serial.println(stateToText(currentState));
}

void updateFinding()
{
    const uint32_t elapsed = millis() - findingStartedAt;

    if (elapsed >= MainConfig::Timing::FIND_DURATION_MS) {
        OutputController::stopWarningOutputs();
        currentState = stateBeforeFinding;
        applyStarterLock();

        Serial.print("[STATE] FIND completed, return to ");
        Serial.println(stateToText(currentState));
        return;
    }

    const bool sirenOn =
        elapsed % MainConfig::Timing::FIND_SIREN_PERIOD_MS <
        MainConfig::Timing::FIND_SIREN_ON_MS;

    const bool turnOn =
        elapsed % MainConfig::Timing::TURN_SIGNAL_PERIOD_MS <
        MainConfig::Timing::TURN_SIGNAL_ON_MS;

    OutputController::setSiren(sirenOn);
    OutputController::setTurnSignal(turnOn);
}

void updateAlarm()
{
    const uint32_t elapsed = millis();

    const bool sirenOn =
        elapsed % MainConfig::Timing::ALARM_SIREN_PERIOD_MS <
        MainConfig::Timing::ALARM_SIREN_ON_MS;

    const bool turnOn =
        elapsed % MainConfig::Timing::TURN_SIGNAL_PERIOD_MS <
        MainConfig::Timing::TURN_SIGNAL_ON_MS;

    OutputController::setSiren(sirenOn);
    OutputController::setTurnSignal(turnOn);
}

}

void begin()
{
    enterDisarmed();
}

void update()
{
    applyStarterLock();

    switch (currentState) {
        case SystemState::FINDING:
            updateFinding();
            break;

        case SystemState::ALARM:
            updateAlarm();
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

}
