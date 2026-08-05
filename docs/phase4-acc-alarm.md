# Mốc 4 – Cảnh báo ACC trái phép

## File thay đổi

```text
firmware/main_controller/config.h
firmware/main_controller/system_types.h
firmware/main_controller/state_machine.h
firmware/main_controller/state_machine.cpp
firmware/main_controller/main_controller.ino
```

## Nguyên lý

- Khi `DISARMED`, ACC không kích hoạt báo động.
- Khi `ARMED`, ACC ON liên tục 300 ms sẽ chuyển sang `ALARM`.
- `ALARM`:
  - relay đề tiếp tục bị khóa;
  - còi hoạt động theo chu kỳ;
  - xi-nhan phải chớp.
- ACC OFF không tự kết thúc ALARM.
- `SILENCE`:
  - tắt còi và xi-nhan;
  - không mở khóa đề;
  - không DISARM.
- Sau khi ACC OFF ổn định 2 giây, trạng thái `SILENCED`
  trở lại `ARMED` để sẵn sàng cho lần cảnh báo tiếp theo.
- `DISARM` luôn đưa hệ thống về trạng thái không can thiệp.

## Chưa có

- MPU6050.
- GPS.
- A7680C/SMS.
- Lưu trạng thái ARM bằng NVS.
