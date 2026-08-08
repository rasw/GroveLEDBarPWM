#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar;

void setup()
{
  bar.begin();

  // Graduation:
  // 5, 10, 20, 30, 40, 50, 60, 70, 80, 100%
  bar.setGreenToRed(true);
  bar.setGraduated(true);
  bar.setLevel(10);
}

void loop()
{
}
