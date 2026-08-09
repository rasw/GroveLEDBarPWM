# GroveLEDBarPWM V1.6.0

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
