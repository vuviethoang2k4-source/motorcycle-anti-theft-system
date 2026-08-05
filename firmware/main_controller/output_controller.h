#pragma once

namespace OutputController {
void begin();
void setStarterLocked(bool locked);
void setSiren(bool enabled);
void setTurnSignal(bool enabled);
void stopWarningOutputs();
bool isStarterLocked();
bool isSirenEnabled();
bool isTurnSignalEnabled();
}
