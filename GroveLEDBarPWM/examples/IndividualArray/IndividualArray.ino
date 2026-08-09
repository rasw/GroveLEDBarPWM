#include <GroveLEDBarPWM.h>

// Your verified pin assignment.
GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();

  uint8_t pattern[10] = {
    5, 20, 40, 60, 80,
    100, 80, 60, 40, 20
  };

  bar.setBrightnessArray(pattern);
}

void loop()
{
}
