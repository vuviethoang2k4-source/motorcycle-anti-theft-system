#pragma once

/*
 * Lưu các thiết lập nhỏ trong bộ nhớ NVS của ESP32.
 *
 * Mốc này chỉ lưu trạng thái chống trộm:
 * - true: ARMED;
 * - false: DISARMED.
 *
 * Không lưu các trạng thái tạm thời như ALARM, FINDING hoặc SILENCED.
 */
namespace StorageManager {

bool begin();

bool isReady();

/*
 * Đọc trạng thái đã lưu.
 * Nếu chưa từng lưu hoặc NVS lỗi, trả về false (DISARMED).
 */
bool loadAntiTheftArmed();

/*
 * Chỉ ghi flash khi giá trị mới khác giá trị đang lưu.
 */
bool saveAntiTheftArmed(bool armed);

}  // namespace StorageManager
