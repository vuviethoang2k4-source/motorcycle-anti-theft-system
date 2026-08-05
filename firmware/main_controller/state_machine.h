#pragma once
#include "protocol.h"

namespace StateMachine {
void begin();
void update();
CommandResult handleCommand(RemoteCommand command);
SystemState getState();
const char *getStateText();
bool isAntiTheftArmed();
}
