#include "GroveLEDBarPWM.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

GroveLEDBarPWM::GroveLEDBarPWM(uint8_t dataPin, uint8_t clockPin)
  : _dataPin(dataPin), _clockPin(clockPin),
    _greenToRed(true), _graduated(false), _transition(false),
    _graduationMin(5), _transitionSpeed(10), _transitionTime(250),
    _level(0), _levelPercent(0), _targetLevel(0), _targetLevelPercent(0),
    _globalBrightness(100), _lastTransitionUpdate(0),
    _flashDefaultSpeed(250),
    _effect(EFFECT_NONE),
    _effectSpeed(150),
    _effectPosition(0),
    _effectForward(true),
    _effectLastUpdate(0),
    _pulseValue(0),
    _pulseRising(true),
    _pulseMinimum(0),
    _pulseMaximum(255),
    _begun(false),
    _autoUpdate(false)
#if defined(ARDUINO_ARCH_ESP32)
    , _autoTaskHandle(nullptr)
#endif
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _brightness[i] = 0;
    _baseBrightness[i] = 0;
    _flashActive[i] = false;
    _flashValue[i] = 0;
    _flashPeak[i] = 255;
    _flashSpeed[i] = 250;
    _flashRising[i] = true;
    _flashLastUpdate[i] = 0;
  }
}

void GroveLEDBarPWM::begin()
{
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  _begun = true;
  clear();

#if defined(ARDUINO_ARCH_ESP32)
  if (_autoUpdate) startAutoUpdateTask();
#endif
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

  // send16() writes the MY9221 channels in physical channel order.
  // Convert each physical channel back to its logical LED so that
  // setGreenToRed() affects the actual displayed direction.
  for (uint8_t ch = 0; ch < LED_COUNT; ++ch) {
    uint8_t logical = _greenToRed ? (LED_COUNT - 1 - ch) : ch;
    uint16_t output;

    // A flashing LED is an output overlay. Its flash value is already
    // an output PWM value, so it must not be scaled by global brightness.
    if (_flashActive[logical]) {
      output = _flashValue[logical];
    } else {
      output = (uint16_t)_brightness[ch] * _globalBrightness / 100U;
    }

    send16(output);
  }

  send16(0);
  send16(0);
  latch();
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

void GroveLEDBarPWM::setGlobalBrightness(uint8_t percent)
{
  if (percent > 100) percent = 100;
  _globalBrightness = percent;
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    if (_flashActive[i]) {
      uint8_t ch = logicalToChannel(i);
      uint8_t target = (uint16_t)_baseBrightness[ch] * _globalBrightness / 100U;
      if (!_flashRising[i] && _flashValue[i] < target) _flashValue[i] = target;
    }
  }
  sendFrame();
}

uint8_t GroveLEDBarPWM::getGlobalBrightness() const
{
  return _globalBrightness;
}

uint8_t GroveLEDBarPWM::graphBrightnessForPercent(uint8_t index, uint8_t percent) const
{
  if (index >= LED_COUNT || percent == 0) return 0;

  uint16_t start = (uint16_t)index * 10U;
  if (percent <= start) return 0;

  uint16_t fraction = percent - start;
  if (fraction >= 10) return targetBrightnessForLogical(index);

  uint8_t full = targetBrightnessForLogical(index);
  return (uint16_t)full * fraction / 10U;
}

void GroveLEDBarPWM::rebuildGraphFromPercent()
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    uint8_t ch = logicalToChannel(i);
    uint8_t v = graphBrightnessForPercent(i, _targetLevelPercent);
    _baseBrightness[ch] = v;
    if (!_flashActive[i]) _brightness[ch] = v;
  }
  _level = (_targetLevelPercent + 9) / 10;
  if (_targetLevelPercent == 0) _level = 0;
  _levelPercent = _targetLevelPercent;
  stopFlashesOutsideLevel();
}

void GroveLEDBarPWM::stopFlashesOutsideLevel()
{
  // A flash is an independent overlay and is allowed even when the normal
  // LED brightness is 0%. Do not cancel active flashes based on the graph
  // brightness.
}

void GroveLEDBarPWM::setLevel(uint8_t level)
{
  if (level > LED_COUNT) level = LED_COUNT;
  setLevelPercent((uint8_t)(level * 10U));
}

void GroveLEDBarPWM::setLED(uint8_t index, uint8_t percent)
{
  if (index >= LED_COUNT)
    return;

  if (percent > 100)
    percent = 100;

  uint8_t ch = logicalToChannel(index);

  // Change only the requested LED.
  _baseBrightness[ch] = (uint16_t)percent * 255U / 100U;
  _brightness[ch] = (uint16_t)_baseBrightness[ch] * _globalBrightness / 100U;

  // Explicitly setting an LED cancels a flash on that LED.
  _flashActive[index] = false;
  _flashValue[index] = _brightness[ch];

  // The existing manual bar.update() performs the MY9221 transmission.
}

void GroveLEDBarPWM::setLevelPercent(uint8_t percent)
{
  if (percent > 100) percent = 100;
  _targetLevelPercent = percent;

  if (!_transition) {
    rebuildGraphFromPercent();
    sendFrame();
  } else {
    // Percentage transitions use a 1% step every 10 ms.
    _lastTransitionUpdate = millis();
  }
}

uint8_t GroveLEDBarPWM::getLevelPercent() const
{
  return _levelPercent;
}

void GroveLEDBarPWM::setTransition(bool enable)
{
  _transition=enable;
  if (!_transition) {
    rebuildGraphFromPercent();
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
bool GroveLEDBarPWM::isTransitioning() const { return _levelPercent != _targetLevelPercent; }


void GroveLEDBarPWM::setAutoUpdate(bool enable)
{
  _autoUpdate = enable;
#if defined(ARDUINO_ARCH_ESP32)
  if (!_begun) return;
  if (enable) startAutoUpdateTask();
  else stopAutoUpdateTask();
#else
  (void)enable;
#endif
}

bool GroveLEDBarPWM::getAutoUpdate() const
{
  return _autoUpdate;
}

bool GroveLEDBarPWM::isAutoUpdateSupported() const
{
#if defined(ARDUINO_ARCH_ESP32)
  return true;
#else
  return false;
#endif
}

#if defined(ARDUINO_ARCH_ESP32)

void GroveLEDBarPWM::autoUpdateTask(void* parameter)
{
  GroveLEDBarPWM* bar = static_cast<GroveLEDBarPWM*>(parameter);
  for (;;) {
    if (bar->_autoUpdate) bar->update();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void GroveLEDBarPWM::startAutoUpdateTask()
{
  if (_autoTaskHandle != nullptr) return;
  TaskHandle_t handle = nullptr;
  if (xTaskCreate(
        GroveLEDBarPWM::autoUpdateTask,
        "GroveLEDBar",
        2048,
        this,
        1,
        &handle) == pdPASS) {
    _autoTaskHandle = static_cast<void*>(handle);
  }
}

void GroveLEDBarPWM::stopAutoUpdateTask()
{
  if (_autoTaskHandle == nullptr) return;
  TaskHandle_t handle = static_cast<TaskHandle_t>(_autoTaskHandle);
  _autoTaskHandle = nullptr;
  vTaskDelete(handle);
}

#endif

void GroveLEDBarPWM::setFlashSpeed(uint16_t ms)
{
  if (ms < 10) ms = 10;
  _flashDefaultSpeed = ms;
}

uint16_t GroveLEDBarPWM::getFlashSpeed() const { return _flashDefaultSpeed; }

void GroveLEDBarPWM::flashLED(uint8_t index)
{
  flashLED(index, _flashDefaultSpeed);
}

void GroveLEDBarPWM::flashLED(uint8_t index, uint16_t milliseconds, uint8_t flashBrightness)
{
  if (index >= LED_COUNT)
    return;

  if (flashBrightness > 100)
    flashBrightness = 100;

  uint8_t ch = logicalToChannel(index);

  // Synchronise the flash value with the LED's CURRENT displayed brightness.
  // This prevents the first update after flashLED() from introducing a
  // transient value into the other channels.
  _flashValue[index] = _brightness[ch];

  _flashPeak[index] = (uint16_t)flashBrightness * 255U / 100U;
  _flashSpeed[index] = milliseconds;
  _flashActive[index] = true;
  _flashRising[index] = true;
  _flashStartTime[index] = millis();
  _flashLastUpdate[index] = _flashStartTime[index];
}

void GroveLEDBarPWM::stopFlashLED(uint8_t index)
{
  if (index >= LED_COUNT) return;
  _flashActive[index] = false;
  _flashPeak[index] = 255;
  _brightness[logicalToChannel(index)] = _baseBrightness[logicalToChannel(index)];
  sendFrame();
}

void GroveLEDBarPWM::stopAllFlashes()
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    _flashActive[i] = false;
    _flashPeak[i] = 255;
    _brightness[logicalToChannel(i)] = _baseBrightness[logicalToChannel(i)];
  }
  sendFrame();
}

bool GroveLEDBarPWM::isFlashing() const
{
  for (uint8_t i = 0; i < LED_COUNT; ++i) if (_flashActive[i]) return true;
  return false;
}

bool GroveLEDBarPWM::isFlashing(uint8_t index) const
{
  return index < LED_COUNT ? _flashActive[index] : false;
}

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

  sendFrame();
}

bool GroveLEDBarPWM::isEffectRunning() const
{
  return _effect != EFFECT_NONE;
}

void GroveLEDBarPWM::update()
{
  bool changed = false;

  if (_transition && _levelPercent != _targetLevelPercent) {
    unsigned long now = millis();
    if (now - _lastTransitionUpdate >= 10) {
      _lastTransitionUpdate = now;
      if (_targetLevelPercent > _levelPercent) ++_levelPercent;
      else --_levelPercent;
      for (uint8_t i = 0; i < LED_COUNT; ++i) {
        uint8_t ch = logicalToChannel(i);
        uint8_t v = graphBrightnessForPercent(i, _levelPercent);
        _baseBrightness[ch] = v;
        if (!_flashActive[i]) _brightness[ch] = v;
      }
      _level = (_levelPercent + 9) / 10;
      if (_levelPercent == 0) _level = 0;
      stopFlashesOutsideLevel();
      changed = true;
    }
  }

  // Rebuild the visible frame from the stored graph/effect base.
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    if (!_flashActive[i]) _brightness[logicalToChannel(i)] = _baseBrightness[logicalToChannel(i)];
  }

  // Independent flash overlays. Each LED has its own timer and speed.
  // Flash timing is based on elapsed time, not PWM step count. This keeps
  // the requested half-cycle duration independent of flash brightness.
  for (uint8_t i = 0; i < LED_COUNT; ++i) {
    if (!_flashActive[i]) continue;

    uint8_t ch = logicalToChannel(i);
    unsigned long now = millis();
    uint16_t duration = _flashSpeed[i];
    if (duration < 10) duration = 10;

    unsigned long elapsed = now - _flashLastUpdate[i];

    // _flashLastUpdate marks the beginning of the current half-cycle.
    // A half-cycle is exactly 'duration' milliseconds.
    if (elapsed >= duration) {
      _flashLastUpdate[i] = now;
      _flashRising[i] = !_flashRising[i];
      elapsed = 0;
      changed = true;
    }

    uint8_t normalValue =
      (uint16_t)_baseBrightness[ch] * _globalBrightness / 100U;
    uint8_t peak = _flashPeak[i];

    if (_flashRising[i]) {
      // Linear interpolation from normal brightness to requested peak.
      uint16_t span = (peak > normalValue) ? (peak - normalValue) : 0;
      _flashValue[i] =
        normalValue + (uint16_t)span * elapsed / duration;
    } else {
      // Linear interpolation from requested peak back to normal brightness.
      uint16_t span = (peak > normalValue) ? (peak - normalValue) : 0;
      _flashValue[i] =
        peak - (uint16_t)span * elapsed / duration;
    }

    _brightness[ch] = _flashValue[i];
  }


  // Effects retain their existing V1.x behaviour.
  if (_effect == EFFECT_MOVING_DOT) {
    unsigned long now = millis();
    if (now - _effectLastUpdate >= _effectSpeed) {
      _effectLastUpdate = now;
      if (_effectForward) {
        if (_effectPosition < LED_COUNT - 1) ++_effectPosition;
        else { _effectForward = false; --_effectPosition; }
      } else {
        if (_effectPosition > 0) --_effectPosition;
        else { _effectForward = true; ++_effectPosition; }
      }
      changed = true;
    }
    _brightness[logicalToChannel(_effectPosition)] = 255;
  }
  else if (_effect == EFFECT_KNIGHT_RIDER) {
    unsigned long now = millis();
    if (now - _effectLastUpdate >= _effectSpeed) {
      _effectLastUpdate = now;
      if (_effectForward) {
        if (_effectPosition < LED_COUNT - 1) ++_effectPosition;
        else { _effectForward = false; --_effectPosition; }
      } else {
        if (_effectPosition > 0) --_effectPosition;
        else { _effectForward = true; ++_effectPosition; }
      }
      changed = true;
    }
    static const uint8_t glowPercent[3] = {100, 60, 30};
    for (int8_t offset=-2; offset<=2; ++offset) {
      int16_t logical=(int16_t)_effectPosition+offset;
      if(logical<0 || logical>=LED_COUNT) continue;
      uint8_t d=(uint8_t)(offset<0?-offset:offset);
      _brightness[logicalToChannel((uint8_t)logical)] = (uint16_t)glowPercent[d]*255U/100U;
    }
  }
  else if (_effect == EFFECT_PULSE) {
    unsigned long now = millis();
    if (now - _effectLastUpdate >= _effectSpeed) {
      _effectLastUpdate = now;
      if (_pulseRising) {
        if (_pulseValue < _pulseMaximum) ++_pulseValue;
        else { _pulseRising=false; if(_pulseValue>_pulseMinimum)--_pulseValue; }
      } else {
        if (_pulseValue > _pulseMinimum) --_pulseValue;
        else { _pulseRising=true; if(_pulseValue<_pulseMaximum)++_pulseValue; }
      }
      changed=true;
    }
    for(uint8_t i=0;i<LED_COUNT;++i) _brightness[i]=_pulseValue;
  }

  if (changed || _effect != EFFECT_NONE || isFlashing()) sendFrame();
}

void GroveLEDBarPWM::setPulseRange(uint8_t minimumPercent, uint8_t maximumPercent)
{
  if (minimumPercent > 100) minimumPercent = 100;
  if (maximumPercent > 100) maximumPercent = 100;

  if (minimumPercent > maximumPercent) {
    uint8_t temp = minimumPercent;
    minimumPercent = maximumPercent;
    maximumPercent = temp;
  }

  _pulseMinimum = (uint16_t)minimumPercent * 255U / 100U;
  _pulseMaximum = (uint16_t)maximumPercent * 255U / 100U;

  if (_pulseValue < _pulseMinimum) _pulseValue = _pulseMinimum;
  if (_pulseValue > _pulseMaximum) _pulseValue = _pulseMaximum;
}

uint8_t GroveLEDBarPWM::getPulseMinimum() const
{
  return (uint16_t)_pulseMinimum * 100U / 255U;
}

uint8_t GroveLEDBarPWM::getPulseMaximum() const
{
  return (uint16_t)_pulseMaximum * 100U / 255U;
}

void GroveLEDBarPWM::setBrightness(uint8_t index,uint8_t brightness)
{
  if(index>=LED_COUNT)return;
  uint8_t ch=logicalToChannel(index);
  _baseBrightness[ch]=brightness;
  if(!_flashActive[index]) _brightness[ch]=brightness;
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
    if(!_flashActive[i]) _brightness[ch]=v;
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
    if(!_flashActive[i]) _brightness[ch]=p[i];
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
  for(uint8_t i=0;i<LED_COUNT;++i){
    _baseBrightness[i]=brightness;
    if(!_flashActive[logicalToChannel(i)]) _brightness[i]=brightness;
  }
  stopFlashesOutsideLevel();
  sendFrame();
}
void GroveLEDBarPWM::setGreenToRed(bool enable)
{
  if(_greenToRed==enable)return;
  _greenToRed=enable;
  if(!_transition){
    rebuildGraphFromPercent();
    sendFrame();
  }
}
bool GroveLEDBarPWM::getGreenToRed() const{return _greenToRed;}
void GroveLEDBarPWM::setGraduated(bool enable)
{
  if(_graduated==enable)return;
  _graduated=enable;
  if(!_transition){
    rebuildGraphFromPercent();
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
  _level=0; _levelPercent=0; _targetLevel=0; _targetLevelPercent=0;
  _effect=EFFECT_NONE;
  for(uint8_t i=0;i<LED_COUNT;++i){
    _brightness[i]=0; _baseBrightness[i]=0; _flashActive[i]=false;
  }
  sendFrame();
}
