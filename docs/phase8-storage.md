# Mốc 8 – Lưu ARM/DISARM bằng Preferences/NVS

## File mới

```text
firmware/main_controller/storage_manager.h
firmware/main_controller/storage_manager.cpp
```

## File thay đổi

```text
firmware/main_controller/main_controller.ino
```

## Dữ liệu được lưu

Namespace:

```text
antitheft
```

Key:

```text
armed
```

Giá trị:

```text
true  = ARMED
false = DISARMED
```

## Quy tắc khôi phục

- Mất nguồn khi `ARMED`, `ALARM`, `SILENCED` hoặc `FINDING`
  nhưng chống trộm đang bật:
  - khởi động lại ở `ARMED`.
- Mất nguồn khi `DISARMED`:
  - khởi động lại ở `DISARMED`.
- Không khôi phục còi đang kêu hoặc quá trình FIND.
- Relay chỉ khóa dây điều khiển đề; việc khôi phục ARMED
  không được dùng để cắt động cơ đang chạy.

## Giảm ghi flash

`saveAntiTheftArmed()` so sánh với giá trị đã lưu và không ghi lại
nếu ARM/DISARM không thay đổi.
