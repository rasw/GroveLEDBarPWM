#include "GroveLEDBarPWM.h"

GroveLEDBarPWM::GroveLEDBarPWM(uint8_t dataPin, uint8_t clockPin)
  : _dataPin(dataPin),
    _clockPin(clockPin),
    _greenToRed(true),
    _graduated(false),
    _transition(false),
    _graduationMin(5),
    _transitionSpeed(10),
    _transitionTime(250),
    _level(0),
    _targetLevel(0),
    _lastTransitionUpdate(0),
    _flashing(false),
    _flashIndex(0),
    _flashOriginalBrightness(0),
    _flashBrightness(0),
    _flashSpeed(250),
    _flashRising(true),
    _lastFlashUpdate(0)
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

void GroveLEDBarPWM::setPins(uint8_t dataPin, uint8_t clockPin)
{
  _dataPin = dataPin;
  _clockPin = clockPin;
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
}

uint8_t GroveLEDBarPWM::getDataPin() const { return _dataPin; }
uint8_t GroveLEDBarPWM::getClockPin() const { return _clockPin; }

void GroveLEDBarPWM::send16(uint16_t data)
{
  for (uint8_t i = 0; i < 16; ++i) {
    digitalWrite(_dataPin, (data & 0x8000U) ? HIGH : LOW);
    digitalWrite(_clockPin, digitalRead(_clockPin) ? LOW : HIGH);
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
  for (uint8_t channel = 0; channel < LED_COUNT; ++channel)
    send16(_brightness[channel]);
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
  static const uint8_t curve[LED_COUNT] =
    {5, 10, 20, 30, 40, 50, 60, 70, 80, 100};

  if (index >= LED_COUNT) return 0;
  return (uint16_t)curve[index] * 255U / 100U;
}

uint8_t GroveLEDBarPWM::targetBrightnessForLogical(uint8_t index) const
{
  if (index >= LED_COUNT) return 0;
  return _graduated ? graduatedBrightness(index) : 255;
}

void GroveLEDBarPWM::setLevel(uint8_t level)
{
  if (level > LED_COUNT) level = LED_COUNT;
  _targetLevel = level;

  if (!_transition) {
    for (uint8_t i = 0; i < LED_COUNT; ++i)
      _brightness[logicalToChannel(i)] =
        (i < _targetLevel) ? targetBrightnessForLogical(i) : 0;

    _level = _targetLevel;
    sendFrame();
    return;
  }

  _lastTransitionUpdate = millis();
}

void GroveLEDBarPWM::setTransition(bool enable)
{
  _transition = enable;

  if (!_transition) {
    for (uint8_t i = 0; i < LED_COUNT; ++i)
      _brightness[logicalToChannel(i)] =
        (i < _targetLevel) ? targetBrightnessForLogical(i) : 0;

    _level = _targetLevel;
    sendFrame();
  }
}

bool GroveLEDBarPWM::getTransition() const { return _transition; }

void GroveLEDBarPWM::setTransitionSpeed(uint8_t speed)
{
  if (speed == 0) speed = 1;
  _transitionSpeed = speed;

  // Keep the time setting approximately in sync.
  _transitionTime = (uint16_t)((255UL * 10UL + speed / 2) / speed);
}

uint8_t GroveLEDBarPWM::getTransitionSpeed() const
{
  return _transitionSpeed;
}

void GroveLEDBarPWM::setTransitionTime(uint16_t milliseconds)
{
  if (milliseconds < 10) milliseconds = 10;

  _transitionTime = milliseconds;

  // Updates occur every approximately 10 ms.
  // Round to the nearest whole brightness count.
  uint32_t speed = (255UL * 10UL + milliseconds / 2) / milliseconds;

  if (speed < 1) speed = 1;
  if (speed > 255) speed = 255;

  _transitionSpeed = (uint8_t)speed;
}

uint16_t GroveLEDBarPWM::getTransitionTime() const
{
  return _transitionTime;
}

bool GroveLEDBarPWM::isTransitioning() const
{
  return _level != _targetLevel;
}

void GroveLEDBarPWM::update()
{
  // Flashing is independent of bar-level transitions.
  if (_flashing) {
    unsigned long now = millis();

    if (now - _lastFlashUpdate >= 10) {
      _lastFlashUpdate = now;

      uint8_t channel = logicalToChannel(_flashIndex);

      // Approximate number of PWM counts per 10 ms required to
      // complete one ramp in _flashSpeed milliseconds.
      uint16_t steps = _flashSpeed / 10;
      if (steps < 1) steps = 1;

      uint16_t amount = 255UL / steps;
      if (amount < 1) amount = 1;

      if (_flashRising) {
        uint16_t next = (uint16_t)_brightness[channel] + amount;

        if (next >= 255) {
          _brightness[channel] = 255;
          _flashRising = false;
        } else {
          _brightness[channel] = (uint8_t)next;
        }
      } else {
        if (_brightness[channel] <= amount ||
            _brightness[channel] <= _flashOriginalBrightness) {
          _brightness[channel] = _flashOriginalBrightness;
          _flashing = false;
        } else {
          _brightness[channel] -= amount;
        }
      }

      sendFrame();
    }
  }

  if (!_transition || _level == _targetLevel) return;

  unsigned long now = millis();
  if (now - _lastTransitionUpdate < 10) return;
  _lastTransitionUpdate = now;

  if (_targetLevel > _level) {
    uint8_t logicalIndex = _level;
    uint8_t channel = logicalToChannel(logicalIndex);
    uint8_t target = targetBrightnessForLogical(logicalIndex);

    uint16_t next = (uint16_t)_brightness[channel] + _transitionSpeed;

    if (next >= target) {
      _brightness[channel] = target;
      ++_level;
    } else {
      _brightness[channel] = (uint8_t)next;
    }
  } else {
    uint8_t logicalIndex = _level - 1;
    uint8_t channel = logicalToChannel(logicalIndex);

    if (_brightness[channel] <= _transitionSpeed) {
      _brightness[channel] = 0;
      --_level;
    } else {
      _brightness[channel] -= _transitionSpeed;
    }
  }

  sendFrame();
}

void GroveLEDBarPWM::setFlashSpeed(uint16_t milliseconds)
{
  if (milliseconds < 10) milliseconds = 10;
  _flashSpeed = milliseconds;
}

uint16_t GroveLEDBarPWM::getFlashSpeed() const
{
  return _flashSpeed;
}

void GroveLEDBarPWM::flashLED(uint8_t index)
{
  if (index >= LED_COUNT) return;

  _flashIndex = index;
  _flashOriginalBrightness = _brightness[logicalToChannel(index)];
  _flashBrightness = _flashOriginalBrightness;
  _flashRising = true;
  _flashing = true;
  _lastFlashUpdate = millis();
}

bool GroveLEDBarPWM::isFlashing() const
{
  return _flashing;
}

void GroveLEDBarPWM::setBrightness(uint8_t index, uint8_t brightness)
{
  if (index >= LED_COUNT) return;
  _brightness[logicalToChannel(index)] = brightness;
  sendFrame();
}

void GroveLEDBarPWM::setBrightnessPercent(uint8_t index, uint8_t percent)
{
  if (index >= LED_COUNT) return;
  if (percent > 100) percent = 100;

  uint8_t brightness = (uint16_t)percent * 255U / 100U;
  setBrightness(index, brightness);
}

void GroveLEDBarPWM::setBrightnessArray(const uint8_t percentages[LED_COUNT])
{
  if (percentages == nullptr) return;

  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    uint8_t percent = percentages[i];
    if (percent > 100) percent = 100;
    _brightness[logicalToChannel(i)] =
        (uint16_t)percent * 255U / 100U;
  }

  sendFrame();
}

void GroveLEDBarPWM::getBrightnessArray(uint8_t percentages[LED_COUNT]) const
{
  if (percentages == nullptr) return;

  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    percentages[i] =
        (uint16_t)_brightness[logicalToChannel(i)] * 100U / 255U;
  }
}

void GroveLEDBarPWM::setBrightnessArrayPWM(const uint8_t pwm[LED_COUNT])
{
  if (pwm == nullptr) return;

  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[logicalToChannel(i)] = pwm[i];
  }

  sendFrame();
}

void GroveLEDBarPWM::getBrightnessArrayPWM(uint8_t pwm[LED_COUNT]) const
{
  if (pwm == nullptr) return;

  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    pwm[i] = _brightness[logicalToChannel(i)];
  }
}

uint8_t GroveLEDBarPWM::getBrightness(uint8_t index) const
{
  if (index >= LED_COUNT) return 0;
  return _brightness[logicalToChannel(index)];
}

uint8_t GroveLEDBarPWM::getBrightnessPercent(uint8_t index) const
{
  if (index >= LED_COUNT) return 0;

  return (uint16_t)_brightness[logicalToChannel(index)] * 100U / 255U;
}

void GroveLEDBarPWM::setAllBrightness(uint8_t brightness)
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) _brightness[i] = brightness;
  sendFrame();
}

void GroveLEDBarPWM::setGreenToRed(bool enable)
{
  if (_greenToRed == enable) return;
  _greenToRed = enable;

  if (!_transition) {
    for (uint8_t i = 0; i < LED_COUNT; ++i)
      _brightness[logicalToChannel(i)] =
        (i < _level) ? targetBrightnessForLogical(i) : 0;
    sendFrame();
  }
}

bool GroveLEDBarPWM::getGreenToRed() const { return _greenToRed; }

void GroveLEDBarPWM::setGraduated(bool enable)
{
  if (_graduated == enable) return;
  _graduated = enable;

  if (!_transition) {
    for (uint8_t i = 0; i < LED_COUNT; ++i)
      _brightness[logicalToChannel(i)] =
        (i < _level) ? targetBrightnessForLogical(i) : 0;
    sendFrame();
  }
}

bool GroveLEDBarPWM::getGraduated() const { return _graduated; }

void GroveLEDBarPWM::setGraduationMin(uint8_t percentage)
{
  if (percentage > 100) percentage = 100;
  _graduationMin = percentage;
}

uint8_t GroveLEDBarPWM::getGraduationMin() const { return _graduationMin; }

void GroveLEDBarPWM::show() { sendFrame(); }

void GroveLEDBarPWM::clear()
{
  _level = 0;
  _targetLevel = 0;
  for (uint8_t i = 0; i < LED_COUNT; ++i) _brightness[i] = 0;
  sendFrame();
}
