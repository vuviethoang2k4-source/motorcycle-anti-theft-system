#include "motion_sensor.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include "config.h"

namespace MotionSensor {
namespace {

Adafruit_MPU6050 mpu;

bool sensorAvailable = false;
bool monitoringEnabled = false;
bool calibrating = false;
bool motionDetected = false;
bool thresholdExceeded = false;

uint32_t lastSampleAt = 0;
uint32_t thresholdExceededSince = 0;

uint16_t baselineSampleCount = 0;
float baselineSumX = 0.0F;
float baselineSumY = 0.0F;
float baselineSumZ = 0.0F;

float baselineX = 0.0F;
float baselineY = 0.0F;
float baselineZ = 9.80665F;
float baselineMagnitude = 9.80665F;

float lastVibration = 0.0F;
float lastTiltDegrees = 0.0F;

float vectorMagnitude(float x, float y, float z)
{
    return sqrtf(x * x + y * y + z * z);
}

void resetDetection()
{
    motionDetected = false;
    thresholdExceeded = false;
    thresholdExceededSince = 0;
    lastVibration = 0.0F;
    lastTiltDegrees = 0.0F;
}

void startBaselineCalibration()
{
    calibrating = true;

    baselineSampleCount = 0;
    baselineSumX = 0.0F;
    baselineSumY = 0.0F;
    baselineSumZ = 0.0F;

    resetDetection();

    Serial.println(
        "[MPU6050] Keep vehicle still while taking baseline.");
}

float calculateTiltDegrees(float x, float y, float z)
{
    const float currentMagnitude =
        vectorMagnitude(x, y, z);

    if (baselineMagnitude < 0.1F ||
        currentMagnitude < 0.1F) {
        return 0.0F;
    }

    float cosineValue =
        (baselineX * x +
         baselineY * y +
         baselineZ * z) /
        (baselineMagnitude * currentMagnitude);

    cosineValue =
        constrain(cosineValue, -1.0F, 1.0F);

    return acosf(cosineValue) *
           180.0F / PI;
}

void processBaselineSample(float x, float y, float z)
{
    baselineSumX += x;
    baselineSumY += y;
    baselineSumZ += z;
    ++baselineSampleCount;

    if (baselineSampleCount <
        MainConfig::Timing::MOTION_BASELINE_SAMPLES) {
        return;
    }

    baselineX =
        baselineSumX / baselineSampleCount;
    baselineY =
        baselineSumY / baselineSampleCount;
    baselineZ =
        baselineSumZ / baselineSampleCount;

    baselineMagnitude =
        vectorMagnitude(
            baselineX,
            baselineY,
            baselineZ);

    calibrating = false;
    resetDetection();

    Serial.print("[MPU6050] Baseline ready: ");
    Serial.print(baselineX, 3);
    Serial.print(", ");
    Serial.print(baselineY, 3);
    Serial.print(", ");
    Serial.println(baselineZ, 3);
}

void processMotionSample(float x, float y, float z)
{
    const uint32_t now = millis();

    const float currentMagnitude =
        vectorMagnitude(x, y, z);

    lastVibration =
        fabsf(currentMagnitude - baselineMagnitude);

    lastTiltDegrees =
        calculateTiltDegrees(x, y, z);

    const bool exceedsThreshold =
        lastVibration >=
            MainConfig::Motion::VIBRATION_THRESHOLD_MS2 ||
        lastTiltDegrees >=
            MainConfig::Motion::TILT_THRESHOLD_DEG;

    if (!exceedsThreshold) {
        thresholdExceeded = false;
        thresholdExceededSince = 0;
        motionDetected = false;
        return;
    }

    if (!thresholdExceeded) {
        thresholdExceeded = true;
        thresholdExceededSince = now;
        motionDetected = false;
        return;
    }

    motionDetected =
        now - thresholdExceededSince >=
        MainConfig::Timing::MOTION_CONFIRM_MS;
}

}  // namespace

bool begin()
{
    /*
     * ESP32 DevKit V1:
     * SDA GPIO21, SCL GPIO22.
     */
    if (!Wire.begin(
            MainConfig::Pin::MPU_SDA,
            MainConfig::Pin::MPU_SCL,
            MainConfig::I2C_FREQUENCY_HZ)) {
        Serial.println(
            "[MPU6050] Cannot initialize I2C.");
        sensorAvailable = false;
        return false;
    }

    if (!mpu.begin(
            MPU6050_I2CADDR_DEFAULT,
            &Wire)) {
        Serial.println(
            "[MPU6050] Sensor not found.");
        sensorAvailable = false;
        return false;
    }

    mpu.setAccelerometerRange(
        MPU6050_RANGE_8_G);
    mpu.setGyroRange(
        MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(
        MPU6050_BAND_21_HZ);

    sensorAvailable = true;
    monitoringEnabled = false;
    calibrating = false;
    resetDetection();

    Serial.println(
        "[MPU6050] Initialized successfully.");

    return true;
}

void setMonitoringEnabled(bool enabled)
{
    if (!sensorAvailable) {
        monitoringEnabled = false;
        calibrating = false;
        resetDetection();
        return;
    }

    if (enabled == monitoringEnabled) {
        return;
    }

    monitoringEnabled = enabled;

    if (monitoringEnabled) {
        startBaselineCalibration();
    } else {
        calibrating = false;
        resetDetection();

        Serial.println(
            "[MPU6050] Monitoring disabled.");
    }
}

void update()
{
    if (!sensorAvailable ||
        !monitoringEnabled) {
        return;
    }

    const uint32_t now = millis();

    if (now - lastSampleAt <
        MainConfig::Timing::MOTION_SAMPLE_INTERVAL_MS) {
        return;
    }

    lastSampleAt = now;

    sensors_event_t acceleration;
    sensors_event_t gyro;
    sensors_event_t temperature;

    if (!mpu.getEvent(
            &acceleration,
            &gyro,
            &temperature)) {
        resetDetection();
        return;
    }

    const float x =
        acceleration.acceleration.x;
    const float y =
        acceleration.acceleration.y;
    const float z =
        acceleration.acceleration.z;

    if (calibrating) {
        processBaselineSample(x, y, z);
        return;
    }

    processMotionSample(x, y, z);
}

bool isAvailable()
{
    return sensorAvailable;
}

bool isCalibrating()
{
    return calibrating;
}

bool isMotionDetected()
{
    return motionDetected;
}

float getLastVibration()
{
    return lastVibration;
}

float getLastTiltDegrees()
{
    return lastTiltDegrees;
}

}  // namespace MotionSensor
