#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar;

void setup()
{
  bar.begin();

  // Green -> Red
  bar.setGreenToRed(true);

  // Graduated brightness
  bar.setGraduated(true);

  bar.setLevel(10);
}

void loop()
{
}
