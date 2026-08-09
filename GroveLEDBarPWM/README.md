# GroveLEDBarPWM V1.2.0

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
