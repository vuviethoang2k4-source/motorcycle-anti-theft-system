#pragma once

#include "protocol.h"

namespace EspNowRemote {

bool begin();
void update();

bool isReady();
bool isBusy();

/*
 * Bắt đầu gửi một lệnh.
 * Hàm không chặn chương trình trong lúc chờ ACK.
 */
bool requestCommand(RemoteCommand command);

/*
 * Trả kết quả ACK một lần rồi xóa cờ kết quả.
 */
bool getLastResult(
    RemoteCommand &command,
    CommandResult &result,
    SystemState &systemState);

}
