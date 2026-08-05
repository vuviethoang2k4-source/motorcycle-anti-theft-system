\# GPIO Pin Mapping



\## Main controller – ESP32 DevKit V1



| Chức năng | GPIO |

|---|---:|

| MPU6050 SDA | 21 |

| MPU6050 SCL | 22 |

| A7680C RX về ESP32 | 16 |

| A7680C TX từ ESP32 | 17 |

| GPS TX từ ESP32 | 12 |

| GPS RX về ESP32 | 13 |

| Điều khiển relay khóa đề | 25 |

| Điều khiển xi-nhan phải | 32 |

| Điều khiển còi | 33 |

| Đọc điện áp ắc quy | 35 |

| Đọc tín hiệu ACC | 34 |



\## Remote – ESP32-C3 SuperMini



| Chức năng | GPIO |

|---|---:|

| Nút bật chống trộm | 3 |

| Nút tắt chống trộm | 4 |

| Nút tìm xe | 5 |

| Nút tắt còi | 6 |

| LED 1 | 7 |

| LED 2 | 8 |

| LED 3 | 10 |

| LED 4 | 20 |



\## Mức logic



\- Các nút remote: nhấn = `LOW`.

\- Tín hiệu ACC qua PC817: ACC bật = `LOW`, ACC tắt = `HIGH`.

\- Relay, còi và xi-nhan: điều khiển mức `HIGH`.

