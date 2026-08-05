# Motorcycle Anti-Theft System

Hệ thống giám sát và chống trộm xe máy sử dụng ESP32, được điều khiển bằng remote ESP-NOW và tin nhắn SMS.

## 1. Chức năng chính

- Bật chế độ chống trộm bằng remote hoặc SMS.
- Tắt chế độ chống trộm bằng remote hoặc SMS.
- Phát hiện rung và nghiêng bằng MPU6050.
- Phát hiện trạng thái khóa điện ACC.
- Khóa mạch điều khiển relay đề khi chống trộm đang bật.
- Cảnh báo bằng còi và nhánh xi-nhan phải.
- Tìm xe trong bãi đỗ.
- Tắt còi bằng remote.
- Gửi cảnh báo và vị trí xe qua SMS.
- Định vị xe bằng GPS.

## 2. Phương thức điều khiển

Hệ thống không sử dụng ứng dụng điện thoại, Blynk hoặc nền tảng cloud.

Hai phương thức điều khiển gồm:

1. Remote ESP32-C3 SuperMini giao tiếp với mạch trung tâm bằng ESP-NOW.
2. Tin nhắn SMS thông qua module A7680C.

## 3. Remote điều khiển

Remote có đúng 4 nút:

1. Bật chống trộm.
2. Tắt chống trộm.
3. Tìm xe trong bãi đỗ.
4. Tắt còi.

## 4. Phần cứng chính

- ESP32 DevKit V1.
- ESP32-C3 SuperMini.
- MPU6050.
- Module GPS.
- Module A7680C VoLTE.
- PC817.
- Relay khóa mạch đề.
- Mạch điều khiển còi.
- Mạch điều khiển xi-nhan phải.
- Bộ nguồn 5 V và 8 V.
- Cầu chì tổng 5 A trên dây dương ắc quy.

## 5. Nguyên tắc an toàn

Khi chế độ chống trộm tắt, hệ thống không can thiệp vào quá trình sử dụng xe.

Khi chế độ chống trộm bật, hệ thống chỉ khóa mạch điều khiển đề trước khi động cơ khởi động.

Hệ thống không cắt ECU, CDI, bơm xăng hoặc làm tắt động cơ khi xe đang chạy.

## 6. Cấu trúc repository

```text
motorcycle-anti-theft-system/
├── firmware/
│   ├── main-controller/
│   │   └── main_controller.ino
│   └── remote-controller/
│       └── remote_controller.ino
├── hardware/
│   └── schematic/
│       └── final/
│           ├── machtrungtam.pdf
│           └── remote.pdf
└── README.md