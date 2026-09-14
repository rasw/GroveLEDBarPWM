# GroveLEDBarPWM V1.9.0

V1.5 adds a stable non-blocking flash overlay. A flashing LED is controlled by a separate flash value, while the normal bar transition continues underneath.

```cpp
bar.setTransition(true);
bar.setTransitionTime(500);
bar.setFlashSpeed(500);

bar.setLevel(3);
bar.setLevel(10);
bar.flashLED(5);
```

Call `bar.update()` from `loop()`.

The flashing LED rises from its current brightness to 100%, then falls toward the current underlying brightness. The overlay uses the greater of the two values, preventing a transition direction change from making the LED flicker.

Verified pin example:

```cpp
GroveLEDBarPWM bar(20, 21);
```


## TEST2

`FlashDuringDirectionChange` starts exactly one flash, then reverses the bar transition while that same flash is still running.

No second `flashLED()` call is made during the reversal.

Expected result:

- LED 5 rises smoothly to 100%.
- The bar transition reverses underneath it.
- LED 5 continues its flash.
- LED 5 returns to the current underlying brightness.
- No second flash occurs.


## TEST3

This test verifies that a flash returns to the LED's CURRENT underlying brightness.

1. LED 5 starts at 30%.
2. LED 5 flashes to 100%.
3. While it is flashing, the underlying LED brightness is changed to 60%.
4. The flash should finish and LED 5 should remain at 60%.

This is the final state-management test before V1.5.0 is committed.


## V1.6 Moving Dot

A non-blocking process indicator that sweeps from logical LED 0 to LED 9 and back.

```cpp
bar.setEffectSpeed(150);
bar.startEffect(GroveLEDBarPWM::EFFECT_MOVING_DOT);
```

Call `bar.update()` from `loop()`. Stop with `bar.stopEffect()`. Stopping restores the underlying bar brightness.


## V1.7 Knight Rider

```cpp
bar.setEffectSpeed(100);
bar.startEffect(GroveLEDBarPWM::EFFECT_KNIGHT_RIDER);
```

The centre LED runs at 100%, adjacent LEDs at 60%, and the next LEDs at 30%. The effect travels to each end and reverses.


### V1.7.0 FIXED

Fixed Knight Rider position advancement. The effect now moves from LED 0 to LED 9 and back instead of remaining at the initial position.


## V1.8 Pulse/Breathe

Start the basic whole-bar pulse:

```cpp
bar.setEffectSpeed(20);
bar.startEffect(GroveLEDBarPWM::EFFECT_PULSE);
```

All ten LEDs fade together from off to full brightness and back again.

The effect is non-blocking and is stopped with:

```cpp
bar.stopEffect();
```

which restores the underlying bar brightness.


## Pulse/Breathe range

The pulse range can be limited to a percentage of full PWM:

```cpp
bar.setPulseRange(10, 80);
bar.setEffectSpeed(20);
bar.startEffect(GroveLEDBarPWM::EFFECT_PULSE);
```

This produces a 10% to 80% breathe cycle. The range values are automatically constrained to 0–100%.


## V1.9 Global Brightness

Set the overall LED output brightness without changing the underlying graph level, individual LED brightness, flash state, or effects:

```cpp
bar.setGlobalBrightness(40);
```

The value is 0–100 percent and defaults to 100%. The setting is applied only when the frame is sent to the MY9221, so the stored display state is preserved.

Read it with:

```cpp
uint8_t brightness = bar.getGlobalBrightness();
```


## V2.0 features

### Percentage graph

Use `setLevelPercent()` for a 0–100% graph with PWM on the partially filled LED:

```cpp
bar.setLevelPercent(63);
```

This can display approximately `██████▎░░░` rather than rounding 63% to six LEDs.

### Multiple independent flashing LEDs

Multiple LEDs can flash simultaneously, each with its own rate:

```cpp
bar.flashLED(3, 500);
bar.flashLED(7, 250);
```

Flashing LEDs reach 100% output brightness and return to their normal globally scaled brightness.
Use `stopFlashLED(index)` or `stopAllFlashes()` to stop flashes explicitly.

### V2.0 compatibility

The V1.9 API remains available. Existing `setLevel()` calls continue to work; `setLevel(6)` is equivalent to `setLevelPercent(60)`.


## V2.0 direction behaviour

The default graph direction is **green to red**:

```cpp
GroveLEDBarPWM bar(2, 3);
bar.setLevelPercent(63);
```

This is equivalent to the library starting with:

```cpp
bar.setGreenToRed(true);
```

Use:

```cpp
bar.setGreenToRed(false);
```

to reverse the graph direction. `setLevelPercent()` and `setLevel()` use the same logical direction.


### V2.0 multiple flashing LEDs

`flashLED(index, milliseconds)` starts a repeating non-blocking flash cycle. Multiple LEDs can flash independently:

```cpp
bar.flashLED(3, 500);
bar.flashLED(7, 250);
```

Each LED repeatedly fades from its normal output brightness to 100% and back to its normal brightness. Use `stopFlashLED(index)` or `stopAllFlashes()` to stop a flash.


## V2.1 Background Update Service

On ESP32 targets, V2.1 can run the LED animation engine from a low-priority FreeRTOS task:

```cpp
bar.begin();
bar.setAutoUpdate(true);

bar.setLevelPercent(63);
bar.flashLED(7, 250);
```

No `bar.update()` call is required in `loop()` while automatic updating is enabled.

Manual updating remains available:

```cpp
bar.setAutoUpdate(false);
```

Then call:

```cpp
bar.update();
```

from `loop()` as in previous versions.

`isAutoUpdateSupported()` can be used to check whether the current platform provides the background service. V2.1 currently enables this service on ESP32 targets.


## V2.1.3 — Independent flash brightness

`flashLED()` now accepts an optional flash-brightness percentage:

```cpp
bar.flashLED(3, 500);       // 100% peak flash brightness
bar.flashLED(3, 500, 50);   // 50% peak flash brightness
```

Flash brightness is independent of the LED's normal brightness. An LED with
a normal brightness of 0% can therefore still flash. The normal brightness is
restored when the flash is stopped.


## V2.1.4 — Flash timing correction

Flash fades now use elapsed-time interpolation. The requested period is no
longer shortened when a low flash brightness is selected.

```cpp
bar.flashLED(0, 1300, 5);    // rise to 5% in 1300 ms, fall to normal in 1300 ms
bar.flashLED(0, 1300, 100);  // rise to 100% in 1300 ms, fall to normal in 1300 ms
bar.flashLED(0, 500);        // 100% peak by default
```


## V2.1.5 — Flash timing correction

Flash timing now uses elapsed milliseconds for each half-cycle instead of
calculating a PWM step from the full 0–255 range. The requested flash period
is therefore independent of the selected flash brightness.

For example:

```cpp
bar.flashLED(0, 500, 10);   // 500 ms rise, 500 ms fall
bar.flashLED(0, 500, 30);   // 500 ms rise, 500 ms fall
bar.flashLED(0, 500, 100);  // 500 ms rise, 500 ms fall
```

The flash peak changes, but the timing does not.


## V2.2.0 — Flash start synchronisation

Based directly on the known-good V2.1.5 baseline. The only functional change
is that `flashLED()` synchronises its initial flash value with the selected
LED's current displayed brightness before enabling the flash state. No changes
are made to `sendFrame()`, the MY9221 clocking, auto-update task, or normal
brightness handling.


## V2.3.0 — setLED()

Added `bar.setLED(index, percent)` to set one logical LED from 0–100%.
The selected LED is changed without changing the other LEDs. Values above
100% are clamped to 100%. Calling setLED() stops a flash on that LED.
No frame is transmitted by setLED(); the existing manual `bar.update()`
continues to control MY9221 updates.
