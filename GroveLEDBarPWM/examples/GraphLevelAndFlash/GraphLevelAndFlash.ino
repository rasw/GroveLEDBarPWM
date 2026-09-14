#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();

  // Show a graph level of six LEDs.
  bar.setLevel(6);

  // Dim the complete display without changing the graph state.
  bar.setGlobalBrightness(40);

  // Flash logical LED 5 at the configured rate.
  bar.setFlashSpeed(250);
  bar.flashLED(5);
}

void loop()
{
  bar.update();
}
