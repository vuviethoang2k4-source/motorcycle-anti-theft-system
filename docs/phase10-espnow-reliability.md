# Mốc 10 – Độ tin cậy ESP-NOW

## File thay đổi

```text
firmware/main_controller/espnow_manager.cpp
firmware/remote_controller/espnow_remote.cpp
```

## Vấn đề được xử lý

Remote gửi lại cùng một gói khi không nhận được ACK.

Nếu mạch trung tâm chạy lại lệnh ở mỗi gói gửi lại:

- FIND có thể bị bắt đầu lại nhiều lần;
- ARM/DISARM bị xử lý lặp;
- trạng thái và phản hồi có thể không nhất quán.

## Cơ chế phía mạch trung tâm

Sau khi xử lý một lệnh, mạch trung tâm lưu:

```text
packetId
command
result
systemState
```

Khi nhận lại đúng `packetId` và `command`:

```text
không thực hiện lệnh lần hai
chỉ gửi lại ACK đã lưu
```

Gói chỉ được kiểm tra trùng sau khi mã xác thực hợp lệ.

## Cơ chế phía remote

- Packet ID được lưu trong Preferences/NVS.
- Sau khi remote mất nguồn, packet ID tiếp tục tăng.
- Khi gửi lại do timeout, remote dùng nguyên gói cũ:
  - cùng packetId;
  - cùng command;
  - cùng authenticationCode.
- Chỉ một lần nhấn nút mới tạo packetId mới.

## Namespace NVS remote

```text
namespace: remote
key: packet
```
