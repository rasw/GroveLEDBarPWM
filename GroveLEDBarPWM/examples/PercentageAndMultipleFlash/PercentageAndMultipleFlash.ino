#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(2, 3);

void setup()
{
  bar.begin();
  bar.setGlobalBrightness(40);
  bar.setLevelPercent(63);

  bar.flashLED(3, 500);
  bar.flashLED(6, 250);
}

void loop()
{
  bar.update();
}
