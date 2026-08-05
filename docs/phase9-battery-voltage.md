# Mốc 9 – Đo điện áp ắc quy

## File thay đổi

```text
firmware/main_controller/config.h
firmware/main_controller/vehicle_inputs.h
firmware/main_controller/vehicle_inputs.cpp
firmware/main_controller/main_controller.ino
```

## Cầu phân áp mà code đang giả sử

```text
Ắc quy -> R18 68 kΩ -> GPIO35 -> R19 12 kΩ -> GND
```

Nếu PCB thực tế chưa đổi R18 sang 68 kΩ, không dùng kết quả điện áp
của code này cho đến khi sửa lại hằng số hoặc phần cứng.

## Thuật toán

- Đọc một mẫu mỗi 50 ms.
- Lấy trung bình 16 mẫu.
- Dùng `analogReadMilliVolts()` để lấy điện áp tại ADC.
- Quy đổi ngược qua cầu phân áp.
- Áp dụng hệ số hiệu chỉnh.

## Hiệu chỉnh

Trong `config.h`:

```cpp
constexpr float CALIBRATION_FACTOR = 1.000F;
```

Ví dụ:

```text
Đồng hồ đo: 12,50 V
Code báo:   12,30 V
```

Khi đó:

```text
CALIBRATION_FACTOR = 12,50 / 12,30 = 1,0163
```

## SMS STATUS

Tin nhắn `STATUS` có thêm:

```text
BATTERY: 12.50 V
```

SMS cảnh báo cũng kèm điện áp nếu bộ lọc đã có dữ liệu.
