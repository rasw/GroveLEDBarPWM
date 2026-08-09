#include <GroveLEDBarPWM.h>

// Your verified pin assignment.
GroveLEDBarPWM bar(20, 21);

unsigned long lastFlash = 0;
uint8_t led = 0;

void setup()
{
  bar.begin();

  // Give the LEDs different starting brightness values.
  uint8_t pattern[10] = {
    10, 20, 30, 40, 50,
    60, 70, 80, 90, 100
  };

  bar.setBrightnessArray(pattern);

  // Approx. 250 ms from original brightness to full brightness.
  bar.setFlashSpeed(250);
}

void loop()
{
  bar.update();

  // Flash a different LED every 2 seconds.
  if (millis() - lastFlash >= 2000 && !bar.isFlashing()) {
    lastFlash = millis();

    bar.flashLED(led);

    ++led;
    if (led >= GroveLEDBarPWM::LED_COUNT) {
      led = 0;
    }
  }
}
