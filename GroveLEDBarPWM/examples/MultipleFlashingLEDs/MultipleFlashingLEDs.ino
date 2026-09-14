#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(2, 3);

void setup()
{
  bar.begin();

  // Percentage graph underneath the flashing indicators.
  bar.setLevelPercent(63);

  // Normal display brightness.
  bar.setGlobalBrightness(40);

  // Independent, repeating flash rates.
  bar.flashLED(3, 500);
  bar.flashLED(7, 250);
}

void loop()
{
  bar.update();
}
