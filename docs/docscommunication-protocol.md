\# Giao thức điều khiển hệ thống



\## 1. Nguồn lệnh



Mạch trung tâm nhận lệnh từ hai nguồn:



1\. Remote ESP32-C3 thông qua ESP-NOW.

2\. Tin nhắn SMS thông qua module A7680C.



Hệ thống không sử dụng ứng dụng điện thoại, Blynk hoặc nền tảng cloud.



\## 2. Danh sách lệnh chung



| Mã lệnh | Tên lệnh | Chức năng |

|---:|---|---|

| 1 | ARM | Bật chế độ chống trộm |

| 2 | DISARM | Tắt chế độ chống trộm |

| 3 | FIND | Tìm xe trong bãi đỗ |

| 4 | SILENCE | Tắt còi nhưng vẫn giữ chế độ chống trộm |

| 5 | STATUS | Yêu cầu trạng thái xe |

| 6 | LOCATION | Yêu cầu vị trí GPS |



Bốn lệnh đầu được sử dụng trên remote.



Hai lệnh `STATUS` và `LOCATION` chỉ được sử dụng qua SMS.



\## 3. Gói tin ESP-NOW



Remote gửi cấu trúc dữ liệu:



```cpp

struct RemoteCommandPacket {

&#x20;   uint32\_t packetId;

&#x20;   uint8\_t command;

&#x20;   uint32\_t authenticationCode;

};

```



Ý nghĩa các trường dữ liệu:



\- `packetId`: số thứ tự của gói tin, dùng để hạn chế xử lý lặp.

\- `command`: mã lệnh từ 1 đến 4.

\- `authenticationCode`: mã xác thực giữa remote và mạch trung tâm.



\## 4. Gói phản hồi ESP-NOW



Mạch trung tâm phản hồi cho remote bằng cấu trúc:



```cpp

struct ControllerResponsePacket {

&#x20;   uint32\_t packetId;

&#x20;   uint8\_t command;

&#x20;   uint8\_t result;

&#x20;   uint8\_t systemState;

};

```



Ý nghĩa các trường dữ liệu:



\- `packetId`: số thứ tự của gói tin được phản hồi.

\- `command`: lệnh mà mạch trung tâm đã nhận.

\- `result`: kết quả thực hiện lệnh.

\- `systemState`: trạng thái hiện tại của hệ thống.



Các giá trị của `result`:



| Giá trị | Ý nghĩa |

|---:|---|

| 0 | Lệnh không hợp lệ |

| 1 | Thực hiện thành công |

| 2 | Lỗi xác thực |

| 3 | Hệ thống đang bận |



\## 5. Trạng thái hệ thống



| Mã trạng thái | Tên trạng thái |

|---:|---|

| 0 | DISARMED |

| 1 | ARMED |

| 2 | ALARM |

| 3 | SILENCED |

| 4 | FINDING |



\## 6. Lệnh SMS



Các tin nhắn điều khiển dự kiến:



```text

ARM

DISARM

FIND

SILENCE

STATUS

LOCATION

```



Hệ thống chỉ xử lý tin nhắn SMS được gửi từ số điện thoại chủ xe đã lưu trong file `secrets.h`.



Tin nhắn gửi từ số điện thoại không hợp lệ sẽ không được thực hiện.



\## 7. Phản hồi SMS



Ví dụ phản hồi khi bật chống trộm:



```text

ANTI-THEFT: ARMED

```



Ví dụ phản hồi khi tắt chống trộm:



```text

ANTI-THEFT: DISARMED

```



Ví dụ phản hồi khi tắt còi:



```text

ALARM: SILENCED

ANTI-THEFT: ARMED

```



Ví dụ phản hồi trạng thái xe:



```text

ACC: OFF

ANTI-THEFT: ARMED

BATTERY: 12.4V

GPS: AVAILABLE

```



Ví dụ phản hồi vị trí GPS:



```text

LOCATION:

https://maps.google.com/?q=21.000000,105.000000

```



\## 8. Nguyên tắc xử lý lệnh



\- Lệnh `ARM` khóa mạch điều khiển đề và bắt đầu giám sát chống trộm.

\- Lệnh `DISARM` tắt cảnh báo, nhả khóa đề và ngừng can thiệp vào hoạt động của xe.

\- Lệnh `FIND` chỉ kích hoạt còi và xi-nhan phải trong một khoảng thời gian ngắn.

\- Lệnh `SILENCE` chỉ tắt còi, không tắt chế độ chống trộm.

\- Lệnh `STATUS` chỉ trả về trạng thái xe, không làm thay đổi trạng thái hệ thống.

\- Lệnh `LOCATION` chỉ trả về vị trí GPS, không làm thay đổi trạng thái hệ thống.

\- Mọi lệnh ESP-NOW phải được kiểm tra địa chỉ MAC và mã xác thực.

\- Mọi lệnh SMS phải được kiểm tra số điện thoại người gửi.

