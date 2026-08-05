# Mốc 6 – GPS UART1 và TinyGPSPlus

## File mới

```text
firmware/main_controller/gps_manager.h
firmware/main_controller/gps_manager.cpp
```

## File thay đổi

```text
firmware/main_controller/config.h
firmware/main_controller/main_controller.ino
```

## Thư viện cần cài

Trong Arduino IDE Library Manager, cài:

```text
TinyGPSPlus
```

Tác giả thư viện: Mikal Hart.

## Sơ đồ kết nối đã chốt

Các nhãn TX/RX được nhìn từ phía ESP32:

```text
ESP32 GPIO12 TX_GPS -> chân RX của GPS
ESP32 GPIO13 RX_GPS <- chân TX của GPS
```

Đầu nối module GPS:

```text
VCC, RX, TX, GND
```

Trong code:

```cpp
gpsSerial.begin(
    9600,
    SERIAL_8N1,
    13,
    12);
```

Thứ tự hai chân cuối là RX trước, TX sau.

## Chức năng

- Đọc liên tục dữ liệu NMEA, không dùng delay chặn.
- Kiểm tra GPS có đang truyền dữ liệu hay không.
- Kiểm tra tọa độ có hợp lệ và còn mới hay không.
- Lấy:
  - vĩ độ;
  - kinh độ;
  - số vệ tinh;
  - tốc độ;
  - tuổi dữ liệu;
  - số ký tự NMEA đã xử lý.
- Tạo đường dẫn Google Maps cho module SMS ở mốc sau.

## Phân biệt hai trạng thái

```text
GPS data = YES, GPS fix = NO
```

Nghĩa là ESP32 đã nhận NMEA nhưng GPS chưa xác định được vị trí.

```text
GPS data = NO
```

Nghĩa là chưa nhận được dữ liệu UART từ GPS hoặc đã ngừng nhận quá lâu.
