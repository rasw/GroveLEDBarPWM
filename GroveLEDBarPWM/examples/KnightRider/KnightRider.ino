#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();

  uint8_t pattern[10] = {
    10, 20, 30, 40, 50,
    60, 70, 80, 90, 100
  };

  bar.setBrightnessArray(pattern);

  bar.setEffectSpeed(100);
  bar.startEffect(GroveLEDBarPWM::EFFECT_KNIGHT_RIDER);
}

void loop()
{
  bar.update();
}
