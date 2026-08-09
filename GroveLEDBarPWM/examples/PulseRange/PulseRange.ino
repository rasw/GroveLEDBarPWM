#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();
  bar.clear();

  // Breathe between 10% and 80%.
  bar.setPulseRange(10, 80);

  // Milliseconds between brightness steps.
  bar.setEffectSpeed(20);

  bar.startEffect(GroveLEDBarPWM::EFFECT_PULSE);
}

void loop()
{
  bar.update();
}
