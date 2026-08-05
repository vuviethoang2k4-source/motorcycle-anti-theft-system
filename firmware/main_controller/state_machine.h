#pragma once

#include "protocol.h"
#include "system_types.h"

namespace StateMachine {

void begin();
void update(const AlarmInputs &inputs);

CommandResult handleCommand(RemoteCommand command);

SystemState getState();
const char *getStateText();

bool isAntiTheftArmed();

/*
 * Nguyên nhân gần nhất làm hệ thống chuyển sang ALARM.
 */
const char *getAlarmReasonText();

}  // namespace StateMachine
