---
category: Developer Reference
version: 2.8.3
---

# Stage Reference

## Core Classes



### Setup and Configuration in main.cpp

```cpp
// In main.cpp
#include "models/DodecaRGBv2r0.h"

CRGB leds[Model::NUM_LEDS];  

void setup() {
    FastLED.addLeds<WS2812B, 19, GRB>(leds, 0, Model::NUM_LEDS/2);
    FastLED.addLeds<WS2812B, 18, GRB>(leds + Model::NUM_LEDS/2, Model::NUM_LEDS/2);
    
    stage.addLeds(leds);
    stage.brightness(128);
    stage.leds.fill(CRGB::Red);
    stage.update();
}
```



### Error Handling Strategy

Since we can't use exceptions on the microcontroller:

1. Array access is bounds-checked but returns safe values:
   - Out of range face index returns last valid face
   - Out of range LED index returns last LED
   - Invalid ring/edge numbers return empty spans

2. Methods return sentinel values:
   - Invalid geometric queries return zero vectors
   - Size queries return 0 for invalid inputs
   - Color operations silently skip invalid LEDs

3. Debug output warnings:
    - when validations fail, warnings are printed to the console
    - Example: "Warning: Face index %d out of range"

This strategy prevents crashes while making issues visible during testing.

