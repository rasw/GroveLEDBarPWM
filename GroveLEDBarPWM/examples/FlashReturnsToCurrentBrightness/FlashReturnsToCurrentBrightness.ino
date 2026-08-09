#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

unsigned long testStart = 0;
bool flashStarted = false;
bool changedBase = false;

void setup()
{
  bar.begin();

  bar.setGreenToRed(true);
  bar.setGraduated(false);

  bar.setTransition(true);
  bar.setFlashSpeed(1000);

  // Start with LED 5 at 30%.
  bar.setBrightnessPercent(5, 30);

  testStart = millis();
}

void loop()
{
  bar.update();

  unsigned long elapsed = millis() - testStart;

  // Start one flash after 1 second.
  if (!flashStarted && elapsed >= 1000) {
    flashStarted = true;
    bar.flashLED(5);
  }

  // While the flash is running, change LED 5's underlying brightness
  // from 30% to 60%.
  if (flashStarted && !changedBase &&
      elapsed >= 1500) {
    changedBase = true;
    bar.setBrightnessPercent(5, 60);
  }
}
