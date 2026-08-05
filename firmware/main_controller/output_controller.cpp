#include "output_controller.h"
#include <Arduino.h>
#include "config.h"

namespace OutputController {
namespace {
bool starterLocked = false;
bool sirenEnabled = false;
bool turnSignalEnabled = false;

void writeConfiguredLevel(
    uint8_t pin,
    bool enabled,
    uint8_t activeLevel,
    uint8_t inactiveLevel)
{
    digitalWrite(pin, enabled ? activeLevel : inactiveLevel);
}
}

void begin()
{
    digitalWrite(
        MainConfig::Pin::STARTER_RELAY,
        MainConfig::Logic::STARTER_LOCK_INACTIVE);
    digitalWrite(
        MainConfig::Pin::SIREN,
        MainConfig::Logic::SIREN_INACTIVE);
    digitalWrite(
        MainConfig::Pin::TURN_SIGNAL,
        MainConfig::Logic::TURN_SIGNAL_INACTIVE);

    pinMode(MainConfig::Pin::STARTER_RELAY, OUTPUT);
    pinMode(MainConfig::Pin::SIREN, OUTPUT);
    pinMode(MainConfig::Pin::TURN_SIGNAL, OUTPUT);

    setStarterLocked(false);
    stopWarningOutputs();
}

void setStarterLocked(bool locked)
{
    starterLocked = locked;
    writeConfiguredLevel(
        MainConfig::Pin::STARTER_RELAY,
        locked,
        MainConfig::Logic::STARTER_LOCK_ACTIVE,
        MainConfig::Logic::STARTER_LOCK_INACTIVE);
}

void setSiren(bool enabled)
{
    sirenEnabled = enabled;
    writeConfiguredLevel(
        MainConfig::Pin::SIREN,
        enabled,
        MainConfig::Logic::SIREN_ACTIVE,
        MainConfig::Logic::SIREN_INACTIVE);
}

void setTurnSignal(bool enabled)
{
    turnSignalEnabled = enabled;
    writeConfiguredLevel(
        MainConfig::Pin::TURN_SIGNAL,
        enabled,
        MainConfig::Logic::TURN_SIGNAL_ACTIVE,
        MainConfig::Logic::TURN_SIGNAL_INACTIVE);
}

void stopWarningOutputs()
{
    setSiren(false);
    setTurnSignal(false);
}

bool isStarterLocked() { return starterLocked; }
bool isSirenEnabled() { return sirenEnabled; }
bool isTurnSignalEnabled() { return turnSignalEnabled; }

}
