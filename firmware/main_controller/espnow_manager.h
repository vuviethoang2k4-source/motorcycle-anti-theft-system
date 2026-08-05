#pragma once

#include "protocol.h"

namespace EspNowManager {

bool begin();
void update();

bool isReady();

/*
 * Trả về true khi có một lệnh hợp lệ mới.
 */
bool getPendingCommand(
    RemoteCommand &command,
    uint32_t &packetId);

/*
 * Trả ACK về remote sau khi mạch trung tâm xử lý lệnh.
 */
bool sendResponse(
    uint32_t packetId,
    RemoteCommand command,
    CommandResult result,
    SystemState currentState);

}
