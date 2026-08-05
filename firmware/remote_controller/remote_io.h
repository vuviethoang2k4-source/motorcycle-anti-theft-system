#pragma once
#include "protocol.h"

namespace RemoteIo {
void begin();
void update();
bool getPendingCommand(RemoteCommand &command);
void showCommandSuccess(RemoteCommand command);
void showCommunicationFailure();
void allLedsOff();
}
