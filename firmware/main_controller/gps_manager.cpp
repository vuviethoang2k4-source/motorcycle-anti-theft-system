#include "gps_manager.h"

#include <Arduino.h>
#include <TinyGPS++.h>

#include "config.h"

namespace GpsManager {
namespace {

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

bool uartStarted = false;
uint32_t lastByteReceivedAt = 0;
uint32_t totalBytesReceived = 0;

}  // namespace

bool begin()
{
    /*
     * Thứ tự HardwareSerial::begin:
     * baud, config, RX pin, TX pin.
     *
     * Vì nhãn trên sơ đồ được nhìn từ phía ESP32:
     * - RX ESP32 = GPIO13;
     * - TX ESP32 = GPIO12.
     */
    gpsSerial.begin(
        MainConfig::GPS_BAUD,
        SERIAL_8N1,
        MainConfig::Pin::GPS_RX,
        MainConfig::Pin::GPS_TX);

    uartStarted = true;
    lastByteReceivedAt = 0;
    totalBytesReceived = 0;

    Serial.print("[GPS] UART1 started at ");
    Serial.print(MainConfig::GPS_BAUD);
    Serial.println(" baud.");

    Serial.print("[GPS] ESP32 RX pin: GPIO");
    Serial.println(MainConfig::Pin::GPS_RX);

    Serial.print("[GPS] ESP32 TX pin: GPIO");
    Serial.println(MainConfig::Pin::GPS_TX);

    return true;
}

void update()
{
    if (!uartStarted) {
        return;
    }

    while (gpsSerial.available() > 0) {
        const char receivedCharacter =
            static_cast<char>(gpsSerial.read());

        gps.encode(receivedCharacter);

        ++totalBytesReceived;
        lastByteReceivedAt = millis();
    }
}

bool isReceivingData()
{
    if (!uartStarted ||
        totalBytesReceived == 0 ||
        lastByteReceivedAt == 0) {
        return false;
    }

    return millis() - lastByteReceivedAt <=
           MainConfig::Timing::GPS_DATA_TIMEOUT_MS;
}

bool hasValidFix()
{
    if (!gps.location.isValid()) {
        return false;
    }

    return gps.location.age() <=
           MainConfig::Timing::GPS_FIX_MAX_AGE_MS;
}

double getLatitude()
{
    return hasValidFix()
               ? gps.location.lat()
               : 0.0;
}

double getLongitude()
{
    return hasValidFix()
               ? gps.location.lng()
               : 0.0;
}

float getSpeedKmph()
{
    return gps.speed.isValid()
               ? static_cast<float>(gps.speed.kmph())
               : 0.0F;
}

uint32_t getSatelliteCount()
{
    return gps.satellites.isValid()
               ? gps.satellites.value()
               : 0;
}

uint32_t getLocationAgeMs()
{
    return gps.location.isValid()
               ? gps.location.age()
               : UINT32_MAX;
}

uint32_t getCharactersProcessed()
{
    return gps.charsProcessed();
}

String getGoogleMapsUrl()
{
    if (!hasValidFix()) {
        return "GPS: NO FIX";
    }

    String url =
        "https://maps.google.com/?q=";

    url += String(gps.location.lat(), 6);
    url += ",";
    url += String(gps.location.lng(), 6);

    return url;
}

}  // namespace GpsManager
