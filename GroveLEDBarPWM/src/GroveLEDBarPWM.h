#ifndef GROVE_LED_BAR_PWM_H
#define GROVE_LED_BAR_PWM_H

#include <Arduino.h>

class GroveLEDBarPWM {
public:
  static constexpr uint8_t LED_COUNT = 10;

  enum Effect {
    EFFECT_NONE = 0,
    EFFECT_MOVING_DOT = 1,
    EFFECT_KNIGHT_RIDER = 2,
    EFFECT_PULSE = 3
  };

  GroveLEDBarPWM(uint8_t dataPin = 8, uint8_t clockPin = 9);

  void begin();
  void setPins(uint8_t dataPin, uint8_t clockPin);
  uint8_t getDataPin() const;
  uint8_t getClockPin() const;

  void setLevel(uint8_t level);
  void setLED(uint8_t index, uint8_t percent);

  void setLevelPercent(uint8_t percent);
  uint8_t getLevelPercent() const;

  // Global output brightness. This scales the display without changing
  // the underlying LED brightness/graph/flash state.
  void setGlobalBrightness(uint8_t percent);
  uint8_t getGlobalBrightness() const;

  void setTransition(bool enable);
  bool getTransition() const;
  void setTransitionSpeed(uint8_t speed);
  uint8_t getTransitionSpeed() const;
  void setTransitionTime(uint16_t milliseconds);
  uint16_t getTransitionTime() const;

  void update();

  // V2.1 optional background animation service.
  // On ESP32 this runs update() from a low-priority FreeRTOS task.
  void setAutoUpdate(bool enable);
  bool getAutoUpdate() const;
  bool isAutoUpdateSupported() const;

  bool isTransitioning() const;

  void setFlashSpeed(uint16_t milliseconds);
  uint16_t getFlashSpeed() const;
  void flashLED(uint8_t index);
  void flashLED(uint8_t index, uint16_t milliseconds, uint8_t flashBrightness = 100);
  void stopFlashLED(uint8_t index);
  void stopAllFlashes();
  bool isFlashing() const;
  bool isFlashing(uint8_t index) const;

  // V1.6 effects
  void setEffectSpeed(uint16_t milliseconds);
  uint16_t getEffectSpeed() const;
  void startEffect(Effect effect);
  void stopEffect();
  bool isEffectRunning() const;

  // Pulse/Breathe controls
  void setPulseRange(uint8_t minimumPercent, uint8_t maximumPercent);
  uint8_t getPulseMinimum() const;
  uint8_t getPulseMaximum() const;

  void setBrightness(uint8_t index, uint8_t brightness);
  void setBrightnessPercent(uint8_t index, uint8_t percent);
  void setBrightnessArray(const uint8_t percentages[LED_COUNT]);
  void getBrightnessArray(uint8_t percentages[LED_COUNT]) const;
  void setBrightnessArrayPWM(const uint8_t pwm[LED_COUNT]);
  void getBrightnessArrayPWM(uint8_t pwm[LED_COUNT]) const;
  uint8_t getBrightness(uint8_t index) const;
  uint8_t getBrightnessPercent(uint8_t index) const;
  void setAllBrightness(uint8_t brightness);

  void setGreenToRed(bool enable);
  bool getGreenToRed() const;
  void setGraduated(bool enable);
  bool getGraduated() const;
  void setGraduationMin(uint8_t percentage);
  uint8_t getGraduationMin() const;

  void show();
  void clear();

private:
  uint8_t _dataPin;
  uint8_t _clockPin;
  uint8_t _brightness[LED_COUNT];
  uint8_t _baseBrightness[LED_COUNT];

  bool _greenToRed;
  bool _graduated;
  bool _transition;
  uint8_t _graduationMin;
  uint8_t _transitionSpeed;
  uint16_t _transitionTime;
  uint8_t _level;
  uint8_t _levelPercent;
  uint8_t _targetLevelPercent;
  uint8_t _globalBrightness;
  uint8_t _targetLevel;
  unsigned long _lastTransitionUpdate;

  // Multiple independent flash overlays.
  bool _flashActive[LED_COUNT];
  uint8_t _flashValue[LED_COUNT];
  uint8_t _flashPeak[LED_COUNT];
  uint16_t _flashSpeed[LED_COUNT];
  bool _flashRising[LED_COUNT];
  unsigned long _flashLastUpdate[LED_COUNT];
  unsigned long _flashStartTime[LED_COUNT];
  uint16_t _flashDefaultSpeed;

  Effect _effect;
  uint16_t _effectSpeed;
  uint8_t _effectPosition;
  bool _effectForward;
  unsigned long _effectLastUpdate;

  // Pulse/Breathe state
  uint8_t _pulseValue;
  bool _pulseRising;
  uint8_t _pulseMinimum;
  uint8_t _pulseMaximum;

  void send16(uint16_t value);
  void latch();
  void sendFrame();
  uint8_t logicalToChannel(uint8_t index) const;
  uint8_t graduatedBrightness(uint8_t index) const;
  uint8_t targetBrightnessForLogical(uint8_t index) const;
  uint8_t graphBrightnessForPercent(uint8_t index, uint8_t percent) const;
  void rebuildGraphFromPercent();
  void stopFlashesOutsideLevel();

  bool _begun;
  bool _autoUpdate;

#if defined(ARDUINO_ARCH_ESP32)
  void* _autoTaskHandle;
  static void autoUpdateTask(void* parameter);
  void startAutoUpdateTask();
  void stopAutoUpdateTask();
#endif
};

#endif
