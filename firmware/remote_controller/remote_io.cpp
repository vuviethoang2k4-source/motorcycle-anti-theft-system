#include "remote_io.h"
#include <Arduino.h>
#include "config.h"

namespace RemoteIo {
namespace {

struct ButtonChannel {
    uint8_t buttonPin;
    uint8_t ledPin;
    RemoteCommand command;
    bool lastRawPressed;
    bool stablePressed;
    uint32_t rawChangedAt;
};

ButtonChannel channels[] = {
    {RemoteConfig::Pin::BUTTON_ARM,
     RemoteConfig::Pin::LED_ARM,
     RemoteCommand::ARM, false, false, 0},

    {RemoteConfig::Pin::BUTTON_DISARM,
     RemoteConfig::Pin::LED_DISARM,
     RemoteCommand::DISARM, false, false, 0},

    {RemoteConfig::Pin::BUTTON_FIND,
     RemoteConfig::Pin::LED_FIND,
     RemoteCommand::FIND, false, false, 0},

    {RemoteConfig::Pin::BUTTON_SILENCE,
     RemoteConfig::Pin::LED_SILENCE,
     RemoteCommand::SILENCE, false, false, 0}
};

RemoteCommand pendingCommand = RemoteCommand::NONE;
uint8_t feedbackLedPin = 255;
uint32_t feedbackLedOffAt = 0;

bool readPressed(uint8_t pin)
{
    return digitalRead(pin) ==
           RemoteConfig::Logic::BUTTON_PRESSED;
}

void writeLed(uint8_t pin, bool enabled)
{
    digitalWrite(
        pin,
        enabled
            ? RemoteConfig::Logic::LED_ON
            : RemoteConfig::Logic::LED_OFF);
}

uint8_t ledForCommand(RemoteCommand command)
{
    for (const ButtonChannel &channel : channels) {
        if (channel.command == command) {
            return channel.ledPin;
        }
    }
    return 255;
}

void startLedFeedback(uint8_t pin)
{
    if (feedbackLedPin != 255) {
        writeLed(feedbackLedPin, false);
    }

    feedbackLedPin = pin;

    if (feedbackLedPin != 255) {
        writeLed(feedbackLedPin, true);
        feedbackLedOffAt =
            millis() + RemoteConfig::Timing::LED_FEEDBACK_MS;
    }
}
}

void begin()
{
    for (ButtonChannel &channel : channels) {
        pinMode(channel.buttonPin, INPUT_PULLUP);
        pinMode(channel.ledPin, OUTPUT);
        writeLed(channel.ledPin, false);

        channel.lastRawPressed =
            readPressed(channel.buttonPin);
        channel.stablePressed =
            channel.lastRawPressed;
        channel.rawChangedAt = millis();
    }

    pendingCommand = RemoteCommand::NONE;
    feedbackLedPin = 255;
}

void update()
{
    const uint32_t now = millis();

    if (feedbackLedPin != 255 &&
        static_cast<int32_t>(now - feedbackLedOffAt) >= 0) {
        writeLed(feedbackLedPin, false);
        feedbackLedPin = 255;
    }

    for (ButtonChannel &channel : channels) {
        const bool rawPressed =
            readPressed(channel.buttonPin);

        if (rawPressed != channel.lastRawPressed) {
            channel.lastRawPressed = rawPressed;
            channel.rawChangedAt = now;
        }

        if (now - channel.rawChangedAt <
            RemoteConfig::Timing::BUTTON_DEBOUNCE_MS) {
            continue;
        }

        if (rawPressed == channel.stablePressed) {
            continue;
        }

        channel.stablePressed = rawPressed;

        if (channel.stablePressed &&
            pendingCommand == RemoteCommand::NONE) {
            pendingCommand = channel.command;
            startLedFeedback(channel.ledPin);
        }
    }
}

bool getPendingCommand(RemoteCommand &command)
{
    if (pendingCommand == RemoteCommand::NONE) {
        return false;
    }

    command = pendingCommand;
    pendingCommand = RemoteCommand::NONE;
    return true;
}

void showCommandSuccess(RemoteCommand command)
{
    startLedFeedback(ledForCommand(command));
}

void showCommunicationFailure()
{
    for (const ButtonChannel &channel : channels) {
        writeLed(channel.ledPin, true);
    }

    delay(150);
    allLedsOff();
}

void allLedsOff()
{
    for (const ButtonChannel &channel : channels) {
        writeLed(channel.ledPin, false);
    }

    feedbackLedPin = 255;
}

}
