#include "storage_manager.h"

#include <Arduino.h>
#include <Preferences.h>

namespace StorageManager {
namespace {

constexpr char NAMESPACE_NAME[] = "antitheft";
constexpr char KEY_ARMED[] = "armed";

Preferences preferences;

bool storageReady = false;
bool cachedArmedState = false;
bool cachedStateValid = false;

}  // namespace

bool begin()
{
    /*
     * false: mở namespace ở chế độ đọc/ghi.
     */
    storageReady =
        preferences.begin(
            NAMESPACE_NAME,
            false);

    if (!storageReady) {
        Serial.println(
            "[STORAGE] Cannot open NVS namespace.");
        return false;
    }

    cachedArmedState =
        preferences.getBool(KEY_ARMED, false);
    cachedStateValid = true;

    Serial.print("[STORAGE] Stored anti-theft state: ");
    Serial.println(
        cachedArmedState ? "ARMED" : "DISARMED");

    return true;
}

bool isReady()
{
    return storageReady;
}

bool loadAntiTheftArmed()
{
    if (!storageReady) {
        return false;
    }

    if (!cachedStateValid) {
        cachedArmedState =
            preferences.getBool(KEY_ARMED, false);
        cachedStateValid = true;
    }

    return cachedArmedState;
}

bool saveAntiTheftArmed(bool armed)
{
    if (!storageReady) {
        Serial.println(
            "[STORAGE] Cannot save because NVS is not ready.");
        return false;
    }

    /*
     * Tránh ghi lại cùng một giá trị nhiều lần,
     * giúp giảm số lần ghi flash không cần thiết.
     */
    if (cachedStateValid &&
        cachedArmedState == armed) {
        return true;
    }

    const size_t bytesWritten =
        preferences.putBool(KEY_ARMED, armed);

    if (bytesWritten == 0) {
        Serial.println(
            "[STORAGE] Failed to save anti-theft state.");
        return false;
    }

    cachedArmedState = armed;
    cachedStateValid = true;

    Serial.print("[STORAGE] Saved anti-theft state: ");
    Serial.println(armed ? "ARMED" : "DISARMED");

    return true;
}

}  // namespace StorageManager
