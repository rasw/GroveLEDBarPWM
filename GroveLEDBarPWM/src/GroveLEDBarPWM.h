#ifndef GROVE_LED_BAR_PWM_H
#define GROVE_LED_BAR_PWM_H

#include <Arduino.h>

class GroveLEDBarPWM {
public:
  static constexpr uint8_t LED_COUNT = 10;

  GroveLEDBarPWM(uint8_t dataPin = 8, uint8_t clockPin = 9);

  void begin();

  void setPins(uint8_t dataPin, uint8_t clockPin);
  uint8_t getDataPin() const;
  uint8_t getClockPin() const;

  void setLevel(uint8_t level);

  // Non-blocking, segment-by-segment transitions.
  void setTransition(bool enable);
  bool getTransition() const;

  // Fine control: brightness counts added/removed approximately every 10 ms.
  void setTransitionSpeed(uint8_t speed);
  uint8_t getTransitionSpeed() const;

  // Easier control: approximate fade time for each LED, in milliseconds.
  void setTransitionTime(uint16_t milliseconds);
  uint16_t getTransitionTime() const;

  void update();
  bool isTransitioning() const;

  void setBrightness(uint8_t index, uint8_t brightness);
  void setBrightnessPercent(uint8_t index, uint8_t percent);

  // Set/read all 10 LEDs as percentage values (0-100).
  void setBrightnessArray(const uint8_t percentages[LED_COUNT]);
  void getBrightnessArray(uint8_t percentages[LED_COUNT]) const;

  // Set/read all 10 LEDs as raw MY9221 PWM values (0-255).
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

  bool _greenToRed;
  bool _graduated;
  bool _transition;

  uint8_t _graduationMin;
  uint8_t _transitionSpeed;
  uint16_t _transitionTime;
  uint8_t _level;
  uint8_t _targetLevel;

  unsigned long _lastTransitionUpdate;

  void send16(uint16_t value);
  void latch();
  void sendFrame();

  uint8_t logicalToChannel(uint8_t index) const;
  uint8_t graduatedBrightness(uint8_t index) const;
  uint8_t targetBrightnessForLogical(uint8_t index) const;
};

#endif
