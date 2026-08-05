# Mốc 5 – MPU6050 và cảnh báo rung/nghiêng

## File mới

```text
firmware/main_controller/motion_sensor.h
firmware/main_controller/motion_sensor.cpp
```

## File thay đổi

```text
firmware/main_controller/config.h
firmware/main_controller/state_machine.cpp
firmware/main_controller/main_controller.ino
```

## Thư viện cần cài

Trong Arduino IDE Library Manager:

```text
Adafruit MPU6050
Adafruit Unified Sensor
Adafruit BusIO
```

## Hoạt động

- MPU6050 dùng I2C:
  - SDA GPIO21;
  - SCL GPIO22.
- Khi hệ thống chuyển từ không giám sát sang giám sát,
  module lấy 75 mẫu để ghi nhận tư thế ban đầu.
- Trong lúc lấy mốc, xe phải đứng yên.
- Phát hiện rung dựa trên độ lệch độ lớn vector gia tốc.
- Phát hiện nghiêng dựa trên góc giữa vector gia tốc hiện tại
  và vector mốc.
- Điều kiện phải vượt ngưỡng liên tục 250 ms.
- Khi `DISARMED`, MPU6050 không được phép kích báo động.
- Khi `ARMED`, rung hoặc nghiêng hợp lệ chuyển hệ thống sang `ALARM`.

## Ngưỡng ban đầu

```cpp
VIBRATION_THRESHOLD_MS2 = 2.2
TILT_THRESHOLD_DEG = 12.0
```

Đây là giá trị khởi đầu và phải hiệu chỉnh trên xe thực tế.
