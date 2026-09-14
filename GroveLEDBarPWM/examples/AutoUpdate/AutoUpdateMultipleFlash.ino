#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(2, 3);

void setup()
{
  bar.begin();
  bar.setAutoUpdate(true);

  bar.setGlobalBrightness(40);
  bar.setLevelPercent(63);

  bar.flashLED(2, 1000);
  bar.flashLED(4, 500);
  bar.flashLED(7, 250);
}

void loop()
{
  // LED animations continue in the background.
  // Simulate application work here if required.
}
