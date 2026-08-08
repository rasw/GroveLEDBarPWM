# GroveLEDBarPWM V1.1.0

V1.1.0 is the stable release with the tuned ten-point graduated brightness curve selected during hardware testing:

**5% → 10% → 20% → 30% → 40% → 50% → 60% → 70% → 80% → 100%**

Verified hardware:
- Grove LED Bar V2.1
- MY9221
- XIAO ESP32-C3
- DATA = GPIO 8
- CLOCK = GPIO 9

## Example

```cpp
GroveLEDBarPWM bar;

void setup() {
  bar.begin();
  bar.setGreenToRed(true);
  bar.setGraduated(true);
  bar.setLevel(10);
}
```

The same graduation reverses correctly when:

```cpp
bar.setGreenToRed(false);
```

Individual brightness control remains available through:

```cpp
bar.setBrightness(index, brightness);
```

where brightness is 0..255.
