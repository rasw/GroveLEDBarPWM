#include <GroveLEDBarPWM.h>

// Your verified pin assignment.
GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();

  // Start the bar with a known state.
  bar.clear();

  // Lower value = faster brightness changes.
  bar.setEffectSpeed(20);
  bar.startEffect(GroveLEDBarPWM::EFFECT_PULSE);
}

void loop()
{
  bar.update();
}
