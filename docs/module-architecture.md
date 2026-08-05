# Cấu trúc firmware đã chốt

## Mạch trung tâm

```text
firmware/main-controller/
├── main_controller.ino
├── config.h
├── protocol.h
├── system_types.h
├── secrets.example.h
├── secrets.h
├── output_controller.h
├── output_controller.cpp
├── vehicle_inputs.h
├── vehicle_inputs.cpp
├── motion_sensor.h
├── motion_sensor.cpp
├── gps_manager.h
├── gps_manager.cpp
├── sms_manager.h
├── sms_manager.cpp
├── espnow_manager.h
├── espnow_manager.cpp
├── state_machine.h
└── state_machine.cpp
```

Các module được thêm lần lượt. Không tạo file rỗng chỉ để đủ cấu trúc.

## Remote

```text
firmware/remote-controller/
├── remote_controller.ino
├── config.h
├── protocol.h
├── secrets.example.h
├── secrets.h
├── remote_io.h
├── remote_io.cpp
├── espnow_remote.h
└── espnow_remote.cpp
```

## Quy tắc

- `.ino` chỉ chứa `setup()`, `loop()` và phần phối hợp cấp cao.
- `.h` là giao diện công khai của module.
- `.cpp` chứa xử lý nội bộ.
- GPIO, mức logic, thời gian và ngưỡng tập trung trong `config.h`.
- `secrets.h` không được đẩy lên GitHub.
- Mỗi module phải biên dịch và kiểm thử trước khi ghép module tiếp theo.
