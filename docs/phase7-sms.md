# Mốc 7 – A7680C và SMS

## File mới

```text
firmware/main_controller/sms_manager.h
firmware/main_controller/sms_manager.cpp
```

## File thay đổi

```text
firmware/main_controller/config.h
firmware/main_controller/state_machine.h
firmware/main_controller/state_machine.cpp
firmware/main_controller/main_controller.ino
```

## UART2

```text
ESP32 GPIO16 RX <- TX A7680C
ESP32 GPIO17 TX -> RX A7680C
Baud rate: 115200
```

## Lệnh AT khởi tạo

```text
AT
ATE0
AT+CMGF=1
AT+CSCS="GSM"
AT+CNMI=2,1,0,0,0
```

Module chạy state machine không chặn toàn bộ `loop()` trong lúc chờ
phản hồi AT.

## Lệnh SMS hợp lệ

```text
ARM
DISARM
FIND
SILENCE
STATUS
LOCATION
```

Chỉ số điện thoại trong `OWNER_PHONE_NUMBER` được chấp nhận.

## Cảnh báo tự động

Khi trạng thái chuyển sang `ALARM`, hệ thống xếp hàng gửi:

```text
THEFT ALERT
REASON: ACC
GPS URL hoặc GPS: NO FIX
```

Tin nhắn chỉ được xếp một lần ở thời điểm chuyển trạng thái.

## secrets.h

Bảo đảm file:

```text
firmware/main_controller/secrets.h
```

có số thật:

```cpp
constexpr char OWNER_PHONE_NUMBER[] = "+84xxxxxxxxx";
```

Không commit `secrets.h`.
