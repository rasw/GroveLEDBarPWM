#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar(20, 21);

unsigned long testStart = 0;
bool started = false;
bool reversed = false;

void setup()
{
  bar.begin();

  bar.setGreenToRed(true);
  bar.setGraduated(true);

  bar.setTransitionTime(1000);
  bar.setTransition(true);

  bar.setFlashSpeed(1000);

  // Start from a low level.
  bar.setLevel(2);

  testStart = millis();
}

void loop()
{
  bar.update();

  unsigned long elapsed = millis() - testStart;

  // At 1 second: start ONE flash while the bar is transitioning upward.
  if (!started && elapsed >= 1000) {
    started = true;

    bar.setLevel(10);
    bar.flashLED(5);
  }

  // At 1500 ms: reverse the bar transition WITHOUT starting another flash.
  if (!reversed && elapsed >= 1500) {
    reversed = true;

    bar.setLevel(2);
  }
}
