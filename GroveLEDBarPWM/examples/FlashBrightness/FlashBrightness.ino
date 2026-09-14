#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(D1, D6);

void setup()
{
  bar.begin();
  bar.setAutoUpdate(true);

  bar.setBrightnessPercent(3, 0);
  bar.flashLED(3, 500);       // 100% by default

  // Or: bar.flashLED(3, 500, 50);
}

void loop()
{
  // No bar.update() required in automatic mode.
}
