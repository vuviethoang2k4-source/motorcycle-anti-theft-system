#pragma once

#include "protocol.h"
#include "system_types.h"

namespace StateMachine {

void begin();

/*
 * Cập nhật trạng thái và mẫu ngõ ra.
 * Hàm này phải được gọi liên tục trong loop().
 */
void update(const AlarmInputs &inputs);

CommandResult handleCommand(RemoteCommand command);

SystemState getState();
const char *getStateText();

bool isAntiTheftArmed();

}  // namespace StateMachine
