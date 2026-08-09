# GroveLEDBarPWM V1.4.0

V1.2 provides configurable GPIOs, graduated brightness, non-blocking segment-by-segment transitions, transition-time control, and individual LED brightness control.

## Pins

Your verified setup can be selected with:

```cpp
GroveLEDBarPWM bar(20, 21);
```

The constructor defaults remain DATA GPIO 8 and CLOCK GPIO 9.

## Individual LED brightness

Raw MY9221 brightness, 0–255:

```cpp
bar.setBrightness(0, 128);
bar.setBrightness(1, 200);
bar.setBrightness(2, 255);
```

Percentage brightness, 0–100%:

```cpp
bar.setBrightnessPercent(0, 5);
bar.setBrightnessPercent(1, 10);
bar.setBrightnessPercent(2, 20);
```

Read the current brightness:

```cpp
uint8_t pwm = bar.getBrightness(0);
uint8_t percent = bar.getBrightnessPercent(0);
```

LED indexes are logical LED positions 0–9 and respect `setGreenToRed()`.

Individual brightness control is independent of graduated mode, so custom brightness patterns can be created.

## Graduation

The tested graduated curve is:

**5%, 10%, 20%, 30%, 40%, 50%, 60%, 70%, 80%, 100%**

## Transitions

```cpp
bar.setTransition(true);
bar.setTransitionTime(500);
bar.setLevel(10);

void loop() {
  bar.update();
}
```

Transitions are non-blocking and occur one LED at a time.


## V1.3 individual array control

Set all ten LEDs with percentage values:

```cpp
uint8_t pattern[10] = {
  5, 20, 40, 60, 80,
  100, 80, 60, 40, 20
};

bar.setBrightnessArray(pattern);
```

Or use raw MY9221 PWM values:

```cpp
uint8_t pwm[10] = {
  13, 51, 102, 153, 204,
  255, 204, 153, 102, 51
};

bar.setBrightnessArrayPWM(pwm);
```

Read the complete current pattern:

```cpp
uint8_t pattern[10];
bar.getBrightnessArray(pattern);
```

or:

```cpp
uint8_t pwm[10];
bar.getBrightnessArrayPWM(pwm);
```

Array indexes are logical LED positions 0-9 and respect `setGreenToRed()`.


## V1.4 individual LED flash

Flash one logical LED from its current brightness to 100% and back:

```cpp
bar.setFlashSpeed(250);
bar.flashLED(4);
```

Call `bar.update()` regularly from `loop()`.

The original LED brightness is saved automatically and restored when the flash finishes. Other LEDs are unaffected.

Check whether a flash is active:

```cpp
if (bar.isFlashing()) {
  // flash still running
}
```

`setFlashSpeed()` specifies the approximate time in milliseconds for the rising/falling ramp.
