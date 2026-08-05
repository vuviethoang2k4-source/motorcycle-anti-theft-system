# Kiểm thử mốc 1

## Mạch trung tâm

1. Chưa nối còi, xi-nhan và dây đề thật.
2. Chọn board `ESP32 Dev Module`.
3. Verify và nạp `main_controller.ino`.
4. Mở Serial Monitor ở 115200.
5. Kiểm tra:
   - `Starter locked: NO`
   - `Siren: OFF`
   - `Turn signal: OFF`
6. Thử ACC:
   - ACC tắt dự kiến in `ACC: OFF`;
   - ACC bật dự kiến in `ACC: ON`.
7. GPIO35 chỉ đọc ADC thô; chưa quy đổi sang volt.

## Remote

1. Chọn board `ESP32C3 Dev Module`.
2. Verify và nạp `remote_controller.ino`.
3. Mở Serial Monitor ở 115200.
4. Nhấn từng nút ARM, DISARM, FIND, SILENCE.
5. Mỗi lần nhấn:
   - Serial chỉ in một lệnh;
   - LED tương ứng sáng ngắn;
   - giữ nút không được lặp lệnh liên tục.

## Commit GitHub

```text
Add modular firmware foundation and IO tests
```
