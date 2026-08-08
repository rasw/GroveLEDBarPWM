#ifndef GROVE_LED_BAR_PWM_H
#define GROVE_LED_BAR_PWM_H

#include <Arduino.h>

class GroveLEDBarPWM {
public:
  static constexpr uint8_t LED_COUNT = 10;

  // Defaults match the verified XIAO ESP32-C3 / Grove LED Bar V2.1 wiring.
  GroveLEDBarPWM(uint8_t dataPin = 8, uint8_t clockPin = 9);

  void begin();

  // Set the number of illuminated logical segments: 0..10.
  void setLevel(uint8_t level);

  // Set one logical LED's brightness: 0..255.
  // LED 0 is the first LED at the current logical end of the bar.
  void setBrightness(uint8_t index, uint8_t brightness);

  // Set all LEDs to the same brightness.
  void setAllBrightness(uint8_t brightness);

  // true  = logical level runs Green -> Red
  // false = logical level runs Red -> Green
  void setGreenToRed(bool enable);

  bool getGreenToRed() const;

  // When enabled, setLevel() applies a graduated brightness profile
  // to the active LEDs. When disabled, active LEDs are full brightness.
  void setGraduated(bool enable);

  bool getGraduated() const;

  // Immediately refresh the MY9221.
  void show();

  // Turn all LEDs off.
  void clear();

private:
  uint8_t _dataPin;
  uint8_t _clockPin;
  uint8_t _brightness[LED_COUNT];
  bool _greenToRed;
  bool _graduated;
  uint8_t _level;

  void send16(uint16_t value);
  void latch();
  void sendFrame();
  uint8_t logicalToChannel(uint8_t index) const;
};

#endif
