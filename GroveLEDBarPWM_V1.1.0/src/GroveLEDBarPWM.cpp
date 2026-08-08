#include "GroveLEDBarPWM.h"

GroveLEDBarPWM::GroveLEDBarPWM(uint8_t dataPin, uint8_t clockPin)
  : _dataPin(dataPin),
    _clockPin(clockPin),
    _greenToRed(true),
    _graduated(false),
    _graduationMin(5),
    _level(0)
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[i] = 0;
  }
}

void GroveLEDBarPWM::begin()
{
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  clear();
}

void GroveLEDBarPWM::send16(uint16_t data)
{
  for (uint8_t i = 0; i < 16; ++i) {
    digitalWrite(_dataPin, (data & 0x8000U) ? HIGH : LOW);

    if (digitalRead(_clockPin)) {
      digitalWrite(_clockPin, LOW);
    } else {
      digitalWrite(_clockPin, HIGH);
    }

    data <<= 1;
  }
}

void GroveLEDBarPWM::latch()
{
  digitalWrite(_dataPin, LOW);
  delayMicroseconds(10);

  for (uint8_t i = 0; i < 4; ++i) {
    digitalWrite(_dataPin, HIGH);
    digitalWrite(_dataPin, LOW);
  }
}

void GroveLEDBarPWM::sendFrame()
{
  send16(0x0000);

  for (uint8_t channel = 0; channel < LED_COUNT; ++channel) {
    send16(_brightness[channel]);
  }

  send16(0x0000);
  send16(0x0000);

  latch();
}

uint8_t GroveLEDBarPWM::logicalToChannel(uint8_t index) const
{
  if (index >= LED_COUNT) return 0;
  return _greenToRed ? (LED_COUNT - 1 - index) : index;
}

uint8_t GroveLEDBarPWM::graduatedBrightness(uint8_t index) const
{
  // The selected ten-point graduation curve:
  // 5, 10, 20, 30, 40, 50, 60, 70, 80, 100 percent.
  //
  // The value is converted to the MY9221's 8-bit range.
  static const uint8_t curve[LED_COUNT] = {
    5, 10, 20, 30, 40, 50, 60, 70, 80, 100
  };

  if (index >= LED_COUNT) {
    return 0;
  }

  return (uint16_t)curve[index] * 255U / 100U;
}

void GroveLEDBarPWM::setLevel(uint8_t level)
{
  if (level > LED_COUNT) level = LED_COUNT;
  _level = level;

  for (uint8_t channel = 0; channel < LED_COUNT; ++channel) {
    _brightness[channel] = 0;
  }

  if (_level == 0) {
    sendFrame();
    return;
  }

  for (uint8_t i = 0; i < _level; ++i) {
    if (_graduated) {
      _brightness[logicalToChannel(i)] = graduatedBrightness(i);
    } else {
      _brightness[logicalToChannel(i)] = 255;
    }
  }

  sendFrame();
}

void GroveLEDBarPWM::setBrightness(uint8_t index, uint8_t brightness)
{
  if (index >= LED_COUNT) return;
  _brightness[logicalToChannel(index)] = brightness;
  sendFrame();
}

void GroveLEDBarPWM::setAllBrightness(uint8_t brightness)
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[i] = brightness;
  }
  sendFrame();
}

void GroveLEDBarPWM::setGreenToRed(bool enable)
{
  if (_greenToRed == enable) return;
  _greenToRed = enable;
  setLevel(_level);
}

bool GroveLEDBarPWM::getGreenToRed() const
{
  return _greenToRed;
}

void GroveLEDBarPWM::setGraduated(bool enable)
{
  if (_graduated == enable) return;
  _graduated = enable;
  setLevel(_level);
}

bool GroveLEDBarPWM::getGraduated() const
{
  return _graduated;
}

void GroveLEDBarPWM::setGraduationMin(uint8_t percentage)
{
  // Retained for API compatibility. V1.1.1 uses the selected
  // ten-point graduation curve, so this value is not used by
  // setLevel() in graduated mode.
  if (percentage > 100) percentage = 100;
  _graduationMin = percentage;

  if (_graduated) {
    setLevel(_level);
  }
}

uint8_t GroveLEDBarPWM::getGraduationMin() const
{
  return _graduationMin;
}

void GroveLEDBarPWM::show()
{
  sendFrame();
}

void GroveLEDBarPWM::clear()
{
  _level = 0;

  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[i] = 0;
  }

  sendFrame();
}
