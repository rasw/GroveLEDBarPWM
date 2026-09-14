#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();
  bar.setLevel(7);
  bar.setGlobalBrightness(30);
}

void loop()
{
  bar.update();
}
