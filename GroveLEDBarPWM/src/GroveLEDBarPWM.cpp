#include "GroveLEDBarPWM.h"

GroveLEDBarPWM::GroveLEDBarPWM(uint8_t dataPin, uint8_t clockPin)
  : _dataPin(dataPin),
    _clockPin(clockPin),
    _greenToRed(true),
    _graduated(false),
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

    // This reproduces the working MY9221 transfer verified on the
    // user's Grove LED Bar V2.1.
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
  // MY9221 command word used by the verified test.
  send16(0x0000);

  // Ten Grove LED Bar channels.
  for (uint8_t channel = 0; channel < LED_COUNT; ++channel) {
    send16(_brightness[channel]);
  }

  // MY9221 channels 10 and 11 are unused by the Grove bar.
  send16(0x0000);
  send16(0x0000);

  latch();
}

uint8_t GroveLEDBarPWM::logicalToChannel(uint8_t index) const
{
  if (index >= LED_COUNT) {
    return 0;
  }

  // Physical mapping verified on the user's V2.1:
  // channel 0 = red, channel 1 = yellow, channels 2..9 = green.
  //
  // greenToRed=true means logical LED 0 starts at the green end.
  return _greenToRed ? (LED_COUNT - 1 - index) : index;
}

void GroveLEDBarPWM::setLevel(uint8_t level)
{
  if (level > LED_COUNT) {
    level = LED_COUNT;
  }

  _level = level;

  // Rebuild the bar from the logical level.
  for (uint8_t channel = 0; channel < LED_COUNT; ++channel) {
    _brightness[channel] = 0;
  }

  if (_level == 0) {
    sendFrame();
    return;
  }

  if (!_graduated) {
    for (uint8_t i = 0; i < _level; ++i) {
      _brightness[logicalToChannel(i)] = 255;
    }
  } else {
    // Graduated profile: the first active LED is dimmest and the
    // final active LED is brightest. This gives a smooth visual
    // progression without changing the fixed LED colours.
    for (uint8_t i = 0; i < _level; ++i) {
      uint8_t value;

      if (_level == 1) {
        value = 255;
      } else {
        value = (uint16_t)(i + 1) * 255U / _level;
      }

      _brightness[logicalToChannel(i)] = value;
    }
  }

  sendFrame();
}

void GroveLEDBarPWM::setBrightness(uint8_t index, uint8_t brightness)
{
  if (index >= LED_COUNT) {
    return;
  }

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
  if (_greenToRed == enable) {
    return;
  }

  _greenToRed = enable;

  // Preserve the current logical level when reversing direction.
  setLevel(_level);
}

bool GroveLEDBarPWM::getGreenToRed() const
{
  return _greenToRed;
}

void GroveLEDBarPWM::setGraduated(bool enable)
{
  if (_graduated == enable) {
    return;
  }

  _graduated = enable;
  setLevel(_level);
}

bool GroveLEDBarPWM::getGraduated() const
{
  return _graduated;
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
