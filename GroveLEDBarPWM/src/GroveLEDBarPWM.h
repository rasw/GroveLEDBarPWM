#ifndef GROVE_LED_BAR_PWM_H
#define GROVE_LED_BAR_PWM_H

#include <Arduino.h>

class GroveLEDBarPWM {
public:
  static constexpr uint8_t LED_COUNT = 10;

  enum Effect {
    EFFECT_NONE = 0,
    EFFECT_MOVING_DOT = 1
  };

  GroveLEDBarPWM(uint8_t dataPin = 8, uint8_t clockPin = 9);

  void begin();
  void setPins(uint8_t dataPin, uint8_t clockPin);
  uint8_t getDataPin() const;
  uint8_t getClockPin() const;

  void setLevel(uint8_t level);

  void setTransition(bool enable);
  bool getTransition() const;
  void setTransitionSpeed(uint8_t speed);
  uint8_t getTransitionSpeed() const;
  void setTransitionTime(uint16_t milliseconds);
  uint16_t getTransitionTime() const;

  void update();
  bool isTransitioning() const;

  void setFlashSpeed(uint16_t milliseconds);
  uint16_t getFlashSpeed() const;
  void flashLED(uint8_t index);
  bool isFlashing() const;

  // V1.6 effects
  void setEffectSpeed(uint16_t milliseconds);
  uint16_t getEffectSpeed() const;
  void startEffect(Effect effect);
  void stopEffect();
  bool isEffectRunning() const;

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
  uint8_t _targetLevel;
  unsigned long _lastTransitionUpdate;

  // Flash overlay state.
  bool _flashing;
  uint8_t _flashIndex;
  uint8_t _flashOriginalBrightness;
  uint8_t _flashValue;
  uint16_t _flashSpeed;
  bool _flashRising;
  unsigned long _flashLastUpdate;

  Effect _effect;
  uint16_t _effectSpeed;
  uint8_t _effectPosition;
  bool _effectForward;
  unsigned long _effectLastUpdate;

  void send16(uint16_t value);
  void latch();
  void sendFrame();
  uint8_t logicalToChannel(uint8_t index) const;
  uint8_t graduatedBrightness(uint8_t index) const;
  uint8_t targetBrightnessForLogical(uint8_t index) const;
};

#endif
