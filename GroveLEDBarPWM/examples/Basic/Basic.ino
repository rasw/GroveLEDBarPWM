#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar;

void setup()
{
  bar.begin();

  // Logical level starts at the green end.
  bar.setGreenToRed(true);

  bar.setLevel(5);
}

void loop()
{
}
