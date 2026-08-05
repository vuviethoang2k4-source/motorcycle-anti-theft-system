# Mốc 2 – ESP-NOW hai chiều

## File mới

Mạch trung tâm:

```text
espnow_manager.h
espnow_manager.cpp
```

Remote:

```text
espnow_remote.h
espnow_remote.cpp
```

## File được thay

- `config.h` ở hai phía;
- `protocol.h` ở hai phía;
- `main_controller.ino`;
- `remote_controller.ino`.

## secrets.h

Mỗi thư mục firmware cần một `secrets.h`.

Có thể sao chép `secrets.example.h` thành `secrets.h`.

Mạch trung tâm điền MAC của remote:

```cpp
constexpr uint8_t REMOTE_MAC_ADDRESS[6] = {
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};
```

Remote điền MAC của mạch trung tâm:

```cpp
constexpr uint8_t MAIN_CONTROLLER_MAC_ADDRESS[6] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66
};
```

Hai phía phải sử dụng cùng:

```cpp
constexpr uint32_t ESPNOW_SHARED_SECRET = 0xA5C37E19UL;
```

## Chức năng mốc 2

- Wi-Fi STA ở cùng kênh 6.
- Remote gửi gói lệnh.
- Mạch trung tâm kiểm tra mã lệnh và mã xác thực.
- Mạch trung tâm gửi ACK.
- Remote chờ ACK không chặn chương trình.
- Không có ACK thì gửi lại tối đa ba lần.
- Chưa điều khiển relay, còi hoặc xi-nhan.
