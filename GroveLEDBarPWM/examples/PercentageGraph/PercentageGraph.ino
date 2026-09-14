#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(2, 3);

void setup()
{
  bar.begin();
  bar.setGlobalBrightness(60);
  bar.setLevelPercent(63);
}

void loop()
{
  bar.update();
}
