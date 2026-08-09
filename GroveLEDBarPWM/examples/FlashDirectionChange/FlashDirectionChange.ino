#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

unsigned long lastAction = 0;
bool highLevel = false;

void setup()
{
  bar.begin();
  bar.setGreenToRed(true);
  bar.setGraduated(true);
  bar.setTransitionTime(500);
  bar.setTransition(true);
  bar.setFlashSpeed(500);
  bar.setLevel(3);
}

void loop()
{
  bar.update();

  if (millis() - lastAction >= 3000) {
    lastAction = millis();

    if (highLevel) {
      bar.setLevel(3);
    } else {
      bar.setLevel(10);
    }

    // Logical LED 5 = sixth LED.
    bar.flashLED(5);

    highLevel = !highLevel;
  }
}
