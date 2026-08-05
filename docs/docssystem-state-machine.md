\# Máy trạng thái hệ thống chống trộm xe máy



\## 1. Các trạng thái chính



\### DISARMED – Chống trộm tắt



\- Relay khóa đề không can thiệp.

\- Xe sử dụng bình thường bằng chìa khóa cơ.

\- Không cảnh báo rung hoặc nghiêng.

\- Không báo động khi ACC bật.

\- Không yêu cầu remote ở gần xe.



\### ARMED – Chống trộm bật



\- Khóa mạch điều khiển đề.

\- Theo dõi tín hiệu ACC.

\- Theo dõi rung và nghiêng từ MPU6050.

\- Chờ lệnh hợp lệ từ remote hoặc SMS.



\### ALARM – Đang báo động



\- Còi 12 V được kích hoạt.

\- Nhánh xi-nhan phải chớp.

\- Gửi tin nhắn SMS cảnh báo.

\- Có thể gửi kèm vị trí GPS.

\- Mạch đề tiếp tục bị khóa.



\### SILENCED – Đã tắt còi



\- Còi ngừng hoạt động.

\- Chế độ chống trộm vẫn được giữ.

\- Mạch đề vẫn bị khóa.

\- Hệ thống tiếp tục giám sát ACC, rung và nghiêng.

\- Nếu tiếp tục phát hiện sự kiện bất thường mới, hệ thống có thể báo động lại.



\### FINDING – Tìm xe trong bãi đỗ



\- Còi phát tín hiệu ngắn.

\- Nhánh xi-nhan phải chớp.

\- Không thay đổi trạng thái chống trộm hiện tại.

\- Không mở hoặc khóa relay đề ngoài trạng thái hiện có.



\## 2. Lệnh từ remote



Remote có đúng 4 nút:



| Nút | Chức năng |

|---|---|

| ARM | Bật chống trộm |

| DISARM | Tắt chống trộm |

| FIND | Tìm xe trong bãi đỗ |

| SILENCE | Tắt còi |



\## 3. Chuyển trạng thái



\### DISARMED → ARMED



Điều kiện:



\- Nhận lệnh ARM hợp lệ từ remote hoặc SMS.



Hành động:



\- Khóa mạch điều khiển đề.

\- Bắt đầu giám sát ACC và MPU6050.



\### ARMED → DISARMED



Điều kiện:



\- Nhận lệnh DISARM hợp lệ từ remote hoặc SMS.



Hành động:



\- Tắt còi và xi-nhan cảnh báo.

\- Ngừng giám sát chống trộm.

\- Nhả khóa mạch điều khiển đề.

\- Xe hoạt động bình thường bằng chìa khóa cơ.



\### ARMED → ALARM



Điều kiện:



\- Phát hiện rung hoặc nghiêng vượt ngưỡng.

\- ACC bật trái phép khi chống trộm đang bật.



Hành động:



\- Giữ khóa mạch đề.

\- Bật còi.

\- Chớp xi-nhan phải.

\- Gửi SMS cảnh báo.



\### ALARM → SILENCED



Điều kiện:



\- Nhận lệnh SILENCE hợp lệ.



Hành động:



\- Tắt còi.

\- Có thể dừng chớp xi-nhan.

\- Không tắt chế độ chống trộm.

\- Không mở khóa đề.



\### ALARM hoặc SILENCED → DISARMED



Điều kiện:



\- Nhận lệnh DISARM hợp lệ.



Hành động:



\- Tắt toàn bộ cảnh báo.

\- Nhả khóa đề.

\- Trở về hoạt động bình thường.



\### Bất kỳ trạng thái nào → FINDING



Điều kiện:



\- Nhận lệnh FIND hợp lệ.



Hành động:



\- Phát tín hiệu còi ngắn.

\- Chớp xi-nhan phải trong thời gian giới hạn.

\- Sau khi hoàn thành, quay lại trạng thái trước đó.



\## 4. Nguyên tắc an toàn



\- Hệ thống chỉ khóa dây điều khiển relay đề.

\- Không cắt ECU, CDI, bơm xăng hoặc nguồn động cơ.

\- Không làm tắt động cơ khi xe đang chạy.

\- Khi chống trộm tắt, hệ thống không can thiệp.

\- Nút SILENCE chỉ tắt còi, không tắt chống trộm.

\- Nút FIND không làm thay đổi trạng thái ARM hoặc DISARM.

