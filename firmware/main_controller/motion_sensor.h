#pragma once

/*
 * Module MPU6050:
 * - tự lấy mốc tư thế khi chế độ chống trộm vừa được bật;
 * - phát hiện rung theo độ lệch độ lớn vector gia tốc;
 * - phát hiện nghiêng theo góc giữa vector trọng lực hiện tại
 *   và vector mốc.
 */
namespace MotionSensor {

bool begin();
void update();

/*
 * Gọi liên tục với trạng thái giám sát hiện tại.
 * Chuyển false -> true sẽ bắt đầu lấy lại mốc tư thế.
 */
void setMonitoringEnabled(bool enabled);

bool isAvailable();
bool isCalibrating();
bool isMotionDetected();

float getLastVibration();
float getLastTiltDegrees();

}  // namespace MotionSensor
