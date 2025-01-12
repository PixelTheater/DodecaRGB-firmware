# Creating Animations

The DodecaRGB firmware can load multiple animations and switch between them. Each animation is like a small shader program - it runs every frame and updates the LED colors based where it is on the display. This happens 50+ times per second, and the addressable LEDs are updated in parallel. The animation framework provides common functionality and patterns for defining animations, handling settings, presets, playlists, color palettes, status messages, and more.

The framework follows a simple pattern: `Animation` (base class) ← `YourAnimation` (implementation) ← `AnimationManager` (controls flow) with `AnimationParams` for configuration.

## Getting Started

Animations work through a simple lifecycle: they're registered with the system, initialized with parameters, and then their `tick()` function is called every frame to update the LEDs. The `tick()` function should be fast and efficient, typically just looping through the LEDs to update their colors. Think of it like a shader - it's called frequently and needs to be performant.

Here's how things are structured in the source tree:

```text
project_root/
├── include/
│   └── animations/
│       └── fadein.h            # Your new animation's header file
└── src/
    ├── animation_builder.cpp   # Register your animation here
    └── animations/
        └── fadein.cpp          # Your new animation's implementation file
```

Steps to register your animation:

```cpp
// In animation_builder.cpp
#include "animations/fadein.h"
REGISTER_ANIMATION("fadein", FadeIn)

// In main.cpp
animation_manager.add("fadein");
```

## Implementing an Animation

Each animation needs a header and implementation file. Here's a basic example:

```cpp
// fadein.h
class FadeIn : public Animation {
  private:
    float speed;
    uint32_t counter = 0;
    CRGBPalette16 palette;

  public:
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "fadein"; }
};
```

```cpp
// fadein.cpp
void FadeIn::init(const AnimParams& params) {
    speed = params.getFloat("speed", 1.0f);
    palette = params.getPalette("palette", RainbowColors_p);
}

void FadeIn::tick() {
    fadeToBlackBy(leds, numLeds(), 40);  // Prevent brightness accumulation
    
    float position = counter * speed;
    for(int i = 0; i < numLeds(); i++) {
        CRGB color = ColorFromPalette(palette, 
            position + i * 16,    // Offset each LED
            sin8(position));      // Modulate brightness
        nblend(leds[i], color, 128);
    }
    counter++;
}

String FadeIn::getStatus() const {
    output.printf("Speed: %.2f Pos: %.1f\n", speed, float(counter * speed));
    output.print(getAnsiColorString(leds[0]));
    return output.get();
}
```

## Working with Parameters

Animation parameters allow customization through key-value storage:

```cpp
void init(const AnimParams& params) {
    // Access parameters with defaults
    float speed = params.getFloat("speed", 1.0f);
    int brightness = params.getInt("brightness", 255);
    CRGBPalette16 pal = params.getPalette("palette", RainbowColors_p);
}

// Set when creating animation
animation_manager.add("myanimation", {
    {"speed", "1.5"},
    {"brightness", "200"}
});
```

## Color Palettes

Three default palettes are available:

- `basePalette`: Rich, saturated colors
- `highlightPalette`: Bright, light colors
- `uniquePalette`: Distinct, high-contrast colors

Helper functions for color management:

```cpp
String colorName = getClosestColorName(CRGB(255, 0, 0));  // Returns "Red"
String ansiColor = getAnsiColorString(CRGB::Red);         // Terminal color
float brightness = get_perceived_brightness(color);
```

## Best Practices

### LED Management

- Access LEDs directly through `leds[]` array
- Framework handles `FastLED.show()` calls
- Use `fadeToBlackBy()` or `nscale8()` to manage brightness
- `nblend()` for safe color mixing

### Animation Flow

- Track time with counters or `millis()`
- Make speed adjustable via parameters
- No `delay()` calls in `tick()`
- Pre-calculate values in `init()`

### Layout Constants

- `numLeds()`: Total LEDs
- `leds_per_side`: LEDs per face
- `num_sides`: Number of faces (12)

Common face-based pattern:

```cpp
void updateFace(int face, CRGB color) {
    int start = face * leds_per_side;
    fill_solid(&leds[start], leds_per_side, color);
}
```

### Status Reporting

Use `getStatus()` instead of Serial prints:

```cpp
String getStatus() const override {
    output.printf("Speed: %.2f\n", speed);
    output.print(getAnsiColorString(leds[0]));
    return output.get();
}
```

See the `Sparkles` and `XYZScanner` animations for more advanced examples.

## Animation Strategies

### Time and Motion

```cpp
class WaveAnimation : public Animation {
private:
    float speed;
    uint32_t counter = 0;
    
public:
    void init(const AnimParams& params) override {
        speed = params.getFloat("speed", 1.0f);  // Adjustable speed
    }
    
    void tick() override {
        float timePosition = counter * speed;  // Smooth time-based motion
        
        for(int i = 0; i < numLeds(); i++) {
            // Create wave pattern using time
            uint8_t brightness = sin8(timePosition + i * 16);
            leds[i] = CRGB(brightness, brightness, brightness);
        }
        counter++;
    }
};
```

### Face-Based Rendering

```cpp
void renderByFace() {
    for(int face = 0; face < num_sides; face++) {
        // Calculate LED range for this face
        int start = face * leds_per_side;
        int end = start + leds_per_side;
        
        // Example: alternate faces between two colors
        CRGB faceColor = (face % 2 == 0) ? CRGB::Red : CRGB::Blue;
        fill_solid(&leds[start], leds_per_side, faceColor);
    }
}
```

### Palette-Based Colors

```cpp
class PaletteWave : public Animation {
private:
    CRGBPalette16 palette;
    uint32_t counter = 0;
    
public:
    void init(const AnimParams& params) override {
        // Get palette parameter or use default
        palette = params.getPalette("palette", RainbowColors_p);
    }
    
    void tick() override {
        // Clear previous frame
        fadeToBlackBy(leds, numLeds(), 40);
        
        for(int i = 0; i < numLeds(); i++) {
            // Use palette to create smooth color transitions
            uint8_t colorIndex = counter + i * 2;
            uint8_t brightness = sin8(counter + i * 4);
            CRGB color = ColorFromPalette(palette, colorIndex, brightness);
            leds[i] = color;
        }
        counter++;
    }
};
```

### Random Effects

The animation framework automatically seeds both the system random number generator and FastLED's random functions during setup() using hardware entropy from the microcontroller, so you don't need to call random.seed() or random16_set_seed() in your animations.

```cpp
void sparkleEffect() {
    // Add random sparkles to each face
    for(int face = 0; face < num_sides; face++) {
        if(random8() < 40) {  // 40/255 chance per face
            int led = (face * leds_per_side) + random8(leds_per_side);
            leds[led] = CRGB::White;
        }
    }
    // Fade all LEDs each frame
    fadeToBlackBy(leds, numLeds(), 64);
}
```

## Coordinate Systems

The DodecaRGB provides several ways to address LEDs spatially:

### Linear Addressing

The LEDs of each PCB are laid out roughly in a spiral pattern, starting in the center and radiating outwards. Each side connects to the next, so all 12 sides are connected to each other in a single array.

Basic sequential access:

```cpp
for(int i = 0; i < numLeds(); i++) {
    leds[i] = CHSV(i * 256/numLeds(), 255, 255);  // Hue varies smoothly across LEDs
}
```

### Face-Based

To help navigate the geometry of the DodecaRGB, there are constants for the number of sides and the number of LEDs per side:

```cpp
for(int face = 0; face < num_sides; face++) {
    int start = face * leds_per_side;
    fill_solid(&leds[start], leds_per_side, ColorFromPalette(uniquePalette, face * 16, 255));
}
```

### 3D Coordinates

Each LED has a mapped position using the `points[]` array. All of the 3D positions of each LED of the DodecaRGB model have been pre-calculated and are available to animation code.

```cpp
// Access x,y,z coordinates of any LED
float x = points[i].x;
float y = points[i].y;
float z = points[i].z;

// Find LEDs near a point in space
for(int i = 0; i < numLeds(); i++) {
    float dist = points[i].distance_to(target_x, target_y, target_z);
    if(dist < threshold) {
        leds[i] = CRGB::White;
    }
}
```

### Spherical Coordinates

With the x,y,z coordinates in place we can use spherical coordidnates to animate objecgts in a spherical space. This is useful for orbital effects (as seen in Blob animation):

```cpp
// Convert angle and elevation to position
float azimuth = counter * 0.01;          // Horizontal angle
float elevation = PI/2;                  // Vertical angle (0=top, PI=bottom)
float radius = sphere_radius;            // Distance from center

// Convert to cartesian
float x = radius * sin(elevation) * cos(azimuth);
float y = radius * sin(elevation) * sin(azimuth);
float z = radius * cos(elevation);

// Light LEDs near this position
for(int i = 0; i < numLeds(); i++) {
    if(points[i].distance_to(x, y, z) < radius) {
        leds[i] = CRGB::Blue;
    }
}
```

See the `Blob` animation for an example of complex orbital movement using spherical coordinates, and `XYZScanner` for cartesian coordinate scanning effects.
