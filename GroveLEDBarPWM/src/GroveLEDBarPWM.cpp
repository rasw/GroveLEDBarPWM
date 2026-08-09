#include "GroveLEDBarPWM.h"

GroveLEDBarPWM::GroveLEDBarPWM(uint8_t dataPin, uint8_t clockPin)
  : _dataPin(dataPin), _clockPin(clockPin),
    _greenToRed(true), _graduated(false), _transition(false),
    _graduationMin(5), _transitionSpeed(10), _transitionTime(250),
    _level(0), _targetLevel(0), _lastTransitionUpdate(0),
    _flashing(false), _flashIndex(0), _flashOriginalBrightness(0),
    _flashValue(0), _flashSpeed(250), _flashRising(true),
    _flashLastUpdate(0),
    _effect(EFFECT_NONE),
    _effectSpeed(150),
    _effectPosition(0),
    _effectForward(true),
    _effectLastUpdate(0)
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[i] = 0;
    _baseBrightness[i] = 0;
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
  send16(0);
  for (uint8_t i = 0; i < LED_COUNT; ++i) send16(_brightness[i]);
  send16(0); send16(0); latch();
}
uint8_t GroveLEDBarPWM::logicalToChannel(uint8_t index) const
{
  return index < LED_COUNT ? (_greenToRed ? LED_COUNT - 1 - index : index) : 0;
}
uint8_t GroveLEDBarPWM::graduatedBrightness(uint8_t index) const
{
  static const uint8_t curve[LED_COUNT] = {5,10,20,30,40,50,60,70,80,100};
  return index < LED_COUNT ? (uint16_t)curve[index] * 255U / 100U : 0;
}
uint8_t GroveLEDBarPWM::targetBrightnessForLogical(uint8_t index) const
{
  return index < LED_COUNT ? (_graduated ? graduatedBrightness(index) : 255) : 0;
}

void GroveLEDBarPWM::setLevel(uint8_t level)
{
  if (level > LED_COUNT) level = LED_COUNT;
  _targetLevel = level;

  if (!_transition) {
    for (uint8_t i=0;i<LED_COUNT;++i) {
      uint8_t ch=logicalToChannel(i);
      uint8_t v=(i<level)?targetBrightnessForLogical(i):0;
      _baseBrightness[ch]=v;
      if (!_flashing || _flashIndex!=i) _brightness[ch]=v;
    }
    _level=level;
    sendFrame();
  } else {
    _lastTransitionUpdate=millis();
  }
}

void GroveLEDBarPWM::setTransition(bool enable)
{
  _transition=enable;
  if (!_transition) {
    for (uint8_t i=0;i<LED_COUNT;++i) {
      uint8_t ch=logicalToChannel(i);
      uint8_t v=(i<_targetLevel)?targetBrightnessForLogical(i):0;
      _baseBrightness[ch]=v;
      if (!_flashing || _flashIndex!=i) _brightness[ch]=v;
    }
    _level=_targetLevel;
    sendFrame();
  }
}
bool GroveLEDBarPWM::getTransition() const { return _transition; }

void GroveLEDBarPWM::setTransitionSpeed(uint8_t speed)
{
  if (!speed) speed=1;
  _transitionSpeed=speed;
  _transitionTime=(uint16_t)((255UL*10UL+speed/2)/speed);
}
uint8_t GroveLEDBarPWM::getTransitionSpeed() const { return _transitionSpeed; }
void GroveLEDBarPWM::setTransitionTime(uint16_t ms)
{
  if (ms<10) ms=10;
  _transitionTime=ms;
  uint32_t speed=(255UL*10UL+ms/2)/ms;
  if(speed<1)speed=1; if(speed>255)speed=255;
  _transitionSpeed=(uint8_t)speed;
}
uint16_t GroveLEDBarPWM::getTransitionTime() const { return _transitionTime; }
bool GroveLEDBarPWM::isTransitioning() const { return _level!=_targetLevel; }

void GroveLEDBarPWM::setFlashSpeed(uint16_t ms)
{
  if (ms<10) ms=10;
  _flashSpeed=ms;
}
uint16_t GroveLEDBarPWM::getFlashSpeed() const { return _flashSpeed; }

void GroveLEDBarPWM::flashLED(uint8_t index)
{
  if(index>=LED_COUNT) return;
  uint8_t ch=logicalToChannel(index);
  _flashIndex=index;
  _flashOriginalBrightness=_baseBrightness[ch];
  _flashValue=_flashOriginalBrightness;
  _flashRising=true;
  _flashing=true;
  _flashLastUpdate=millis();
  _brightness[ch]=_flashValue;
  sendFrame();
}
bool GroveLEDBarPWM::isFlashing() const { return _flashing; }

void GroveLEDBarPWM::setEffectSpeed(uint16_t milliseconds)
{
  if (milliseconds < 20) milliseconds = 20;
  _effectSpeed = milliseconds;
}

uint16_t GroveLEDBarPWM::getEffectSpeed() const
{
  return _effectSpeed;
}

void GroveLEDBarPWM::startEffect(Effect effect)
{
  _effect = effect;
  _effectPosition = 0;
  _effectForward = true;
  _effectLastUpdate = millis();
  sendFrame();
}

void GroveLEDBarPWM::stopEffect()
{
  _effect = EFFECT_NONE;
  _effectLastUpdate = 0;
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[i] = _baseBrightness[i];
  }
  if (_flashing) {
    uint8_t ch = logicalToChannel(_flashIndex);
    if (_flashValue > _brightness[ch]) _brightness[ch] = _flashValue;
  }
  sendFrame();
}

bool GroveLEDBarPWM::isEffectRunning() const
{
  return _effect != EFFECT_NONE;
}

void GroveLEDBarPWM::update()
{
  bool changed=false;

  // Underlying bar transition.
  if(_transition && _level!=_targetLevel) {
    unsigned long now=millis();
    if(now-_lastTransitionUpdate>=10) {
      _lastTransitionUpdate=now;
      if(_targetLevel>_level) {
        uint8_t li=_level, ch=logicalToChannel(li);
        uint8_t target=targetBrightnessForLogical(li);
        uint16_t next=(uint16_t)_baseBrightness[ch]+_transitionSpeed;
        if(next>=target){_baseBrightness[ch]=target; ++_level;}
        else _baseBrightness[ch]=(uint8_t)next;
      } else {
        uint8_t li=_level-1, ch=logicalToChannel(li);
        if(_baseBrightness[ch]<=_transitionSpeed){_baseBrightness[ch]=0; --_level;}
        else _baseBrightness[ch]-=_transitionSpeed;
      }
      changed=true;
    }
  }

  // Start visible frame from base.
  for(uint8_t i=0;i<LED_COUNT;++i) _brightness[i]=_baseBrightness[i];

  // Flash overlay uses a separate value. This prevents the transition from
  // fighting the flash and causing flicker.
  if(_flashing) {
    unsigned long now=millis();
    if(now-_flashLastUpdate>=10) {
      _flashLastUpdate=now;
      uint8_t ch=logicalToChannel(_flashIndex);

      uint16_t step=(uint16_t)((255UL*10UL+_flashSpeed/2)/_flashSpeed);
      if(step<1) step=1;

      if(_flashRising) {
        uint16_t next=(uint16_t)_flashValue+step;
        if(next>=255) {
          _flashValue=255;
          _flashRising=false;
        } else {
          _flashValue=(uint8_t)next;
        }
      } else {
        // Return to the CURRENT base brightness.
        uint8_t target=_baseBrightness[ch];

        if(_flashValue<=target || _flashValue<=step) {
          _flashValue=target;
          _flashing=false;
        } else {
          uint16_t next=(uint16_t)_flashValue-step;
          if(next<=target) {
            _flashValue=target;
            _flashing=false;
          } else {
            _flashValue=(uint8_t)next;
          }
        }
      }
      changed=true;
    }

    uint8_t ch=logicalToChannel(_flashIndex);
    // Overlay is the greater of the current base and flash value.
    // Therefore a changing base can never make the LED flicker down.
    _brightness[ch]=(_flashValue>_baseBrightness[ch]) ?
                     _flashValue : _baseBrightness[ch];

    // Once the flash value has fallen to/below the base, the overlay is done.
    if(!_flashRising && _flashValue<=_baseBrightness[ch]) {
      _brightness[ch]=_baseBrightness[ch];
      _flashing=false;
    }
  }

  // Moving Dot overlay. It never modifies the underlying bar state.
  if (_effect == EFFECT_MOVING_DOT) {
    unsigned long now = millis();

    if (now - _effectLastUpdate >= _effectSpeed) {
      _effectLastUpdate = now;

      if (_effectForward) {
        if (_effectPosition < LED_COUNT - 1) {
          ++_effectPosition;
        } else {
          _effectForward = false;
          --_effectPosition;
        }
      } else {
        if (_effectPosition > 0) {
          --_effectPosition;
        } else {
          _effectForward = true;
          ++_effectPosition;
        }
      }
      changed = true;
    }

    uint8_t dotChannel = logicalToChannel(_effectPosition);
    _brightness[dotChannel] = 255;
  }

  else if (_effect == EFFECT_KNIGHT_RIDER) {
    unsigned long now = millis();

    // Move the Knight Rider position using the same timing model
    // as the Moving Dot effect.
    if (now - _effectLastUpdate >= _effectSpeed) {
      _effectLastUpdate = now;

      if (_effectForward) {
        if (_effectPosition < LED_COUNT - 1) {
          ++_effectPosition;
        } else {
          _effectForward = false;
          --_effectPosition;
        }
      } else {
        if (_effectPosition > 0) {
          --_effectPosition;
        } else {
          _effectForward = true;
          ++_effectPosition;
        }
      }

      changed = true;
    }

    // Graduated glow: centre 100%, adjacent 60%, next 30%.
    static const uint8_t glowPercent[3] = {100, 60, 30};

    for (int8_t offset = -2; offset <= 2; ++offset) {
      int16_t logical = (int16_t)_effectPosition + offset;
      if (logical < 0 || logical >= LED_COUNT) continue;

      uint8_t distance = (uint8_t)(offset < 0 ? -offset : offset);
      uint8_t value = (uint16_t)glowPercent[distance] * 255U / 100U;
      uint8_t channel = logicalToChannel((uint8_t)logical);

      // Stand-alone effect test: the effect controls these LEDs.
      _brightness[channel] = value;
    }
  }

  if (changed || _effect != EFFECT_NONE) sendFrame();
}

void GroveLEDBarPWM::setBrightness(uint8_t index,uint8_t brightness)
{
  if(index>=LED_COUNT)return;
  uint8_t ch=logicalToChannel(index);
  _baseBrightness[ch]=brightness;
  if(!_flashing || _flashIndex!=index) _brightness[ch]=brightness;
  sendFrame();
}
void GroveLEDBarPWM::setBrightnessPercent(uint8_t index,uint8_t percent)
{
  if(index>=LED_COUNT)return;
  if(percent>100)percent=100;
  setBrightness(index,(uint16_t)percent*255U/100U);
}
void GroveLEDBarPWM::setBrightnessArray(const uint8_t p[LED_COUNT])
{
  if(!p)return;
  for(uint8_t i=0;i<LED_COUNT;++i){
    uint8_t v=(uint16_t)((p[i]>100?100:p[i]))*255U/100U;
    uint8_t ch=logicalToChannel(i); _baseBrightness[ch]=v;
    if(!_flashing || _flashIndex!=i)_brightness[ch]=v;
  }
  sendFrame();
}
void GroveLEDBarPWM::getBrightnessArray(uint8_t p[LED_COUNT]) const
{
  if(!p)return;
  for(uint8_t i=0;i<LED_COUNT;++i)p[i]=(uint16_t)_brightness[logicalToChannel(i)]*100U/255U;
}
void GroveLEDBarPWM::setBrightnessArrayPWM(const uint8_t p[LED_COUNT])
{
  if(!p)return;
  for(uint8_t i=0;i<LED_COUNT;++i){
    uint8_t ch=logicalToChannel(i); _baseBrightness[ch]=p[i];
    if(!_flashing || _flashIndex!=i)_brightness[ch]=p[i];
  }
  sendFrame();
}
void GroveLEDBarPWM::getBrightnessArrayPWM(uint8_t p[LED_COUNT]) const
{
  if(!p)return;
  for(uint8_t i=0;i<LED_COUNT;++i)p[i]=_brightness[logicalToChannel(i)];
}
uint8_t GroveLEDBarPWM::getBrightness(uint8_t index) const
{
  return index<LED_COUNT?_brightness[logicalToChannel(index)]:0;
}
uint8_t GroveLEDBarPWM::getBrightnessPercent(uint8_t index) const
{
  return index<LED_COUNT?(uint16_t)_brightness[logicalToChannel(index)]*100U/255U:0;
}
void GroveLEDBarPWM::setAllBrightness(uint8_t brightness)
{
  for(uint8_t i=0;i<LED_COUNT;++i){_baseBrightness[i]=brightness;_brightness[i]=brightness;}
  sendFrame();
}
void GroveLEDBarPWM::setGreenToRed(bool enable)
{
  if(_greenToRed==enable)return; _greenToRed=enable;
  if(!_transition){
    for(uint8_t i=0;i<LED_COUNT;++i){
      uint8_t ch=logicalToChannel(i),v=(i<_level)?targetBrightnessForLogical(i):0;
      _baseBrightness[ch]=v;
      if(!_flashing || _flashIndex!=i)_brightness[ch]=v;
    }
    sendFrame();
  }
}
bool GroveLEDBarPWM::getGreenToRed() const{return _greenToRed;}
void GroveLEDBarPWM::setGraduated(bool enable)
{
  if(_graduated==enable)return; _graduated=enable;
  if(!_transition){
    for(uint8_t i=0;i<LED_COUNT;++i){
      uint8_t ch=logicalToChannel(i),v=(i<_level)?targetBrightnessForLogical(i):0;
      _baseBrightness[ch]=v;
      if(!_flashing || _flashIndex!=i)_brightness[ch]=v;
    }
    sendFrame();
  }
}
bool GroveLEDBarPWM::getGraduated() const{return _graduated;}
void GroveLEDBarPWM::setGraduationMin(uint8_t percentage)
{
  _graduationMin=percentage>100?100:percentage;
}
uint8_t GroveLEDBarPWM::getGraduationMin() const{return _graduationMin;}
void GroveLEDBarPWM::show(){sendFrame();}
void GroveLEDBarPWM::clear()
{
  _level=0;_targetLevel=0;_flashing=false;
  _effect=EFFECT_NONE;
  for(uint8_t i=0;i<LED_COUNT;++i){_brightness[i]=0;_baseBrightness[i]=0;}
  sendFrame();
}
