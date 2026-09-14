#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(2, 3);

void setup()
{
  bar.begin();

  // V2.1 background animation service.
  bar.setAutoUpdate(true);

  bar.setLevelPercent(63);
  bar.flashLED(7, 250);
}

void loop()
{
  // No bar.update() required.
  // Your normal application code runs here.
}
