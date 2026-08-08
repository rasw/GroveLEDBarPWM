# GroveLEDBarPWM V1.0.0

A lightweight Arduino library for the Seeed Grove LED Bar V2.1 using its MY9221 LED driver.

## Verified hardware

Tested with:
- XIAO ESP32-C3
- Grove LED Bar V2.1
- DATA = GPIO 8
- CLOCK = GPIO 9

## Verified channel mapping

| MY9221 channel | Physical LED |
|---:|---|
| 0 | Red |
| 1 | Yellow |
| 2 | Green |
| 3 | Green |
| 4 | Green |
| 5 | Green |
| 6 | Green |
| 7 | Green |
| 8 | Green |
| 9 | Green |
| 10 | Unused |
| 11 | Unused |

## Basic use

```cpp
#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar;

void setup() {
  bar.begin();
  bar.setGreenToRed(true);
  bar.setLevel(5);
}

void loop() {
}
```

## Graduated brightness

```cpp
#include <GroveLEDBarPWM.h>

GroveLEDBarPWM bar;

void setup() {
  bar.begin();
  bar.setGreenToRed(true);
  bar.setGraduated(true);
  bar.setLevel(10);
}

void loop() {
}
```

`setGraduated(true)` makes the active logical LEDs progressively brighter, while `setGraduated(false)` makes all active LEDs full brightness.

## Individual brightness

```cpp
bar.setBrightness(0, 64);   // 25%
bar.setBrightness(1, 128);  // 50%
bar.setBrightness(2, 192);  // 75%
bar.setBrightness(3, 255);  // 100%
```

The index is a logical index and follows the current `setGreenToRed()` direction.

## Notes

V1 uses the MY9221 transfer protocol verified experimentally with the Grove LED Bar V2.1. The library intentionally does not depend on the Seeed Grove LED Bar library.
