#include <GroveLEDBarPWM.h>

// Your verified pin assignment.
GroveLEDBarPWM bar(20, 21);

void setup()
{
  bar.begin();

  // Individual percentage brightness.
  bar.setBrightnessPercent(0, 5);
  bar.setBrightnessPercent(1, 10);
  bar.setBrightnessPercent(2, 20);
  bar.setBrightnessPercent(3, 30);
  bar.setBrightnessPercent(4, 40);
  bar.setBrightnessPercent(5, 50);
  bar.setBrightnessPercent(6, 60);
  bar.setBrightnessPercent(7, 70);
  bar.setBrightnessPercent(8, 80);
  bar.setBrightnessPercent(9, 100);
}

void loop()
{
}
