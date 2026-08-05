#pragma once

#include <Arduino.h>

/*
 * Các yêu cầu SMS mà chương trình chính có thể xử lý.
 */
enum class SmsRequestType : uint8_t {
    NONE = 0,
    ARM,
    DISARM,
    FIND,
    SILENCE,
    STATUS,
    LOCATION
};

/*
 * Module A7680C/SMS:
 * - UART2 RX GPIO16, TX GPIO17;
 * - khởi tạo bằng AT command không chặn loop lâu;
 * - chỉ nhận lệnh từ OWNER_PHONE_NUMBER;
 * - có hàng đợi gửi SMS.
 */
namespace SmsManager {

bool begin();
void update();

bool isReady();
bool isBusy();

/*
 * Trả về một yêu cầu SMS hợp lệ rồi xóa yêu cầu khỏi hàng đợi.
 */
bool getPendingRequest(SmsRequestType &request);

/*
 * Đưa một tin nhắn vào hàng đợi gửi tới OWNER_PHONE_NUMBER.
 */
bool queueMessage(const String &message);

uint32_t getReceivedSmsCount();
uint32_t getSentSmsCount();
uint32_t getFailedSmsCount();

}  // namespace SmsManager
