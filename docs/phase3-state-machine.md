# Mốc 3 – Máy trạng thái và ngõ ra

## File mới

```text
firmware/main_controller/state_machine.h
firmware/main_controller/state_machine.cpp
```

## File thay đổi

```text
firmware/main_controller/config.h
firmware/main_controller/main_controller.ino
```

## Hoạt động

- `DISARM`: không khóa đề, tắt còi và xi-nhan.
- `ARM`: khóa dây điều khiển relay đề.
- `FIND`: còi ngắn và xi-nhan phải chớp trong 6 giây, sau đó quay lại trạng thái trước.
- `SILENCE`: tắt còi/xi-nhan nhưng không tự DISARM.
- Chưa có ACC alarm, MPU6050, GPS, SMS hoặc lưu NVS.
