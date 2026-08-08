#ifndef GROVE_LED_BAR_PWM_H
#define GROVE_LED_BAR_PWM_H

#include <Arduino.h>

class GroveLEDBarPWM {
public:
  static constexpr uint8_t LED_COUNT = 10;

  GroveLEDBarPWM(uint8_t dataPin = 8, uint8_t clockPin = 9);

  void begin();

  void setLevel(uint8_t level);
  void setBrightness(uint8_t index, uint8_t brightness);
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
  uint8_t _graduationMin;
  uint8_t _level;

  void send16(uint16_t value);
  void latch();
  void sendFrame();
  uint8_t logicalToChannel(uint8_t index) const;
  uint8_t graduatedBrightness(uint8_t index) const;
};

#endif
