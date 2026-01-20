#ifndef VOID_HAL_H
#define VOID_HAL_H

#include <Arduino.h>
#include <LilyGoLib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * VOID-HAL (Hardware Abstraction Layer)
 * Centralizes all hardware access and enforces thread-safety via I2C mutex.
 * Replaces direct calls to 'instance' and manual i2c_lock usage.
 */

class VOID_HAL {
public:
  static void begin();
  static void loop();

  // Haptics
  static void vibrate(uint32_t effect = 1);
  static void setHapticEffect(uint8_t effect);

  // Battery/Power
  static float getBatteryVoltage();
  static int getBatteryPercent();
  static void refreshPower();

  // Display
  static void setBrightness(uint8_t level);

  // Locks (exposed for legacy wrap if needed, but HAL methods should be
  // preferred)
  static void lock();
  static void unlock();

  // Raw access if absolutely necessary
  static LilyGoLoRaPager &get_instance();

private:
  static SemaphoreHandle_t _i2c_mutex;
};

#endif // VOID_HAL_H
