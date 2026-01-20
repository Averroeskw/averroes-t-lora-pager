#include "void_hal.h"

// Access the global instance defined in the LilyGo library
extern LilyGoLoRaPager &instance;

SemaphoreHandle_t VOID_HAL::_i2c_mutex = NULL;

void VOID_HAL::begin() {
  if (!_i2c_mutex) {
    _i2c_mutex = xSemaphoreCreateRecursiveMutex();
  }

  lock();
  instance.begin();
  unlock();
}

void VOID_HAL::loop() {
  lock();
  instance.loop();
  unlock();
}

void VOID_HAL::lock() {
  if (_i2c_mutex) {
    xSemaphoreTakeRecursive(_i2c_mutex, portMAX_DELAY);
  }
}

void VOID_HAL::unlock() {
  if (_i2c_mutex) {
    xSemaphoreGiveRecursive(_i2c_mutex);
  }
}

void VOID_HAL::vibrate(uint32_t effect) {
  lock();
  instance.setHapticEffects(effect);
  instance.vibrator();
  unlock();
}

void VOID_HAL::setHapticEffect(uint8_t effect) {
  lock();
  instance.setHapticEffects(effect);
  unlock();
}

float VOID_HAL::getBatteryVoltage() {
  lock();
  float v = instance.gauge.getVoltage() / 1000.0;
  unlock();
  return v;
}

int VOID_HAL::getBatteryPercent() {
  lock();
  int p = instance.gauge.getStateOfCharge();
  unlock();
  return p;
}

void VOID_HAL::refreshPower() {
  lock();
  instance.gauge.refresh();
  unlock();
}

void VOID_HAL::setBrightness(uint8_t level) {
  lock();
  instance.setBrightness(level);
  unlock();
}

LilyGoLoRaPager &VOID_HAL::get_instance() { return instance; }
