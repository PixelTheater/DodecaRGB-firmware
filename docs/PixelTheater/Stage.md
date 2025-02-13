---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Stage and PixelSurface

## Overview

The Stage provides smart arrays for LED manipulation that feel natural to FastLED users while adding convenient features.

## Usage Examples

```cpp
class SimpleScene : public Scene {
    void tick() override {
        // leds[] works just like FastLED but with built-in methods
        leds.fill(CRGB::Black);
        leds.fadeToBlackBy(20);
        leds[100] = CRGB::Red;  // Direct access still works
        
        // Iterate like a normal array
        for(auto& led : leds) {
            led = CRGB::Blue;
        }
        
        // model.sides[] gives access to faces
        for(auto& side : model.sides) {
            side = CHSV(random8(), 255, 255);  // Fill whole face
            
            // Each face has semantic regions
            side.center = CRGB::Red;
            side.top_row.fill_rainbow(0, 32);
            
            // Rings are indexed
            side.ring[0] = CRGB::Blue;  // Inner ring
            side.ring[1].fadeToBlackBy(20);
        }
    }
};
```

## Smart Arrays

The Stage provides several smart array types that act like normal arrays but with extra features:

### LedArray (leds[])
```cpp
// Regular array access
leds[0] = CRGB::Red;
CRGB color = leds[100];

// Built-in FastLED operations
leds.fill(CRGB::Black);
leds.fadeToBlackBy(20);
leds.blur2d(32);

// Range-based for loops
for(auto& led : leds) {
    led = CRGB::Blue;
}

// Get size
int count = leds.size();
```

### FaceArray (model.sides[])
```cpp
// Access faces by index
model.sides[0] = CRGB::Red;  // Fill whole face

// Iterate faces
for(auto& face : model.sides) {
    face.center = CHSV(random8(), 255, 255);
}

// Face regions
face.center      // Center LEDs
face.top_row     // Top edge
face.ring[0]     // First ring
face.edge.north  // North edge

// Built-in methods work on regions
face.top_row.fill_rainbow(0, 32);
face.ring[1].fadeToBlackBy(20);
```

## Implementation

Under the hood, these are implemented as template classes that wrap arrays but provide iteration and FastLED-style methods:

```cpp
// Simplified implementation sketch
template<typename T>
class SmartArray {
    T* _data;
    size_t _size;
    
public:
    // Array access
    T& operator[](size_t i) { return _data[i]; }
    
    // Iteration
    T* begin() { return _data; }
    T* end() { return _data + _size; }
    
    // FastLED-style methods
    void fill(const T& value);
    void fadeToBlackBy(uint8_t amount);
    // etc...
};

// Usage
using LedArray = SmartArray<CRGB>;
using FaceArray = SmartArray<Face>;
```

## Benefits

1. Familiar array syntax
2. Built-in FastLED operations
3. Range-based for loop support
4. No need to track array sizes
5. Semantic access to regions
6. Method chaining possible
7. Type safety

## Example Scene

```cpp
void tick() override {
    // Fill all faces with different colors
    for(auto& face : model.sides) {
        // Get face index from pointer math
        int idx = &face - &model.sides[0];
        face = CHSV(idx * 32, 255, 255);
        
        // Create gradient in rings
        for(int i = 0; i < face.num_rings; i++) {
            face.ring[i].fadeToBlackBy(i * 32);
        }
        
        // Sparkle the edges
        if(random8() < 32) {
            face.edge.random() = CRGB::White;
        }
    }
    
    // Global effects
    leds.fadeToBlackBy(20);
    leds.blur2d(32);
}
```

## The Model

Represents the physical object (like a dodecahedron):

```cpp
// Basic properties
model.num_sides        // How many faces
model.leds_per_side   // LEDs on each face
model.total_leds      // Total LED count

// Access faces directly
model.face[3].center      // Center LEDs of face 3
model.face[3].top_row    // Top row of face 3
model.face[3].rings[0]   // First ring of LEDs

// Get coordinates
auto p = model.point[led_index];  // Get x,y,z for any LED
float x = p.x, y = p.y, z = p.z;

// Find LEDs by location
auto nearby = model.find_nearby(point, radius);
auto closest = model.find_nearest(x, y, z);
```

## The LED Surface 

Direct control of all LEDs, FastLED-style:

```cpp
// Direct LED access (just like FastLED)
leds[i] = CRGB::Red;
leds(x, y, z) = CRGB::Blue;  // Set by coordinate

// Fill regions
leds.fill_face(3, CRGB::Red);
leds.fill_region(model.face[2].center, CRGB::Blue);

// Common effects
leds.clear();
leds.fade_all(20);
leds.blur2d(32);
leds.radial_fade(center_point, 2.0);
```

## Hardware Setup

```cpp
// Configure LED strips in setup()
stage.add_strip(19, 0, 624);     // Pin 19, first half
stage.add_strip(18, 624, 624);   // Pin 18, second half

// Optional strip config
stage.set_brightness(64);
stage.set_max_power(5000, 5.0);  // 5A at 5V
```

## Implementation Notes

- Model geometry comes from pick-and-place files via utils/
- Points.h defines the mapping of LEDs to coordinates
- LED surface wraps FastLED but adds convenience
- Focus on making common patterns easy to write
- Add helpers based on what animations need most

## Future Considerations

- Support for different model types (cube, sphere, etc)
- More sophisticated geometry queries (intersections, paths)
- Additional animation helpers
- Optimization for common patterns
- Debug visualization support
