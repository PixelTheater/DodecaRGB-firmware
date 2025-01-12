# Creating Animations

The DodecaRGB firmware can load multiple animations and switch between them. Each animation is like a small shader program - it runs every frame and updates the LED colors based on its logic. The animation framework provides common functionality for handling settings, presets, status messages, playlists, and more.

Here's a diagram of the animation framework:
```
+-------------------+     +-------------------+
|  AnimationParams  |     | AnimationManager  |
|  (Parameters)     |     | (Controls Flow)   |
+-------------------+     +-------------------+
         ▲                         ▲
         |                         |
         |                         |
+-------------------+             |
|     Animation     |<------------+
|  (Base Class)     |
+-------------------+
         ▲
         |
+-------------------+
| Your Animation    |
| (e.g. FadeIn)    |
+-------------------+
         ▲
         |
+-------------------+
| AnimationBuilder  |
| (Factory Class)   |
+-------------------+
```

# Creating Animations

Animations work through a simple lifecycle: they're registered with the system, initialized with parameters, and then their `tick()` function is called every frame to update the LEDs. The `tick()` function should be fast and efficient, typically just looping through the LEDs to update their colors. Think of it like a shader - it's called frequently and needs to be performant.

Animations are defined in the `include/animations` directory, and are built around the `Animation` class.

## Creating a new animation

Let's assume the animation is called "FadeIn".

1. Create a new `.cpp` file at `src/animations/fadein.cpp`
2. Create a new `.h` file at `include/animations/fadein.h`
3. Add the animation to the `AnimationBuilder` class in `src/animation_builder.cpp`.

### Example:

Assuming the animation is called "FadeIn", the source files would be `src/animations/fadein.cpp`, `include/animations/fadein.h`.

Then register your animation in `src/animation_builder.cpp`:
```cpp
// at top of file, include your header:
#include "animations/fadein.h"

// at the end of the file, register by name and class:
REGISTER_ANIMATION("fadein", FadeIn)

```

In `src/main.cpp`, you can now add your animation to the list of animations:
```cpp
animation_manager.add("fadein");
```

And for viewing the status of the animation, you can call `animation_manager.getCurrentAnimation()->getStatus()`.

## Implementing the animation

The `Animation` class is an abstract class, so you need to implement the `init()`, `tick()`, and `getStatus()` methods.

### Example:

The header file`include/animations/fadein.h`:
```cpp
#pragma once
#include "animation.h"

class FadeIn : public Animation {
  private:
    int fade_in_time = 1000;
    int current_time = 0;
    bool fading_in = true;

  public:
    FadeIn() = default;  // default constructor
    void init(const AnimParams& params) override;  // initialize the animation
    void tick() override;  // update the animation
    String getStatus() const override;  // get the status of the animation
    const char* getName() const override { return "fadein"; }  // return the name of the animation
}
```

For the `tick()` method, you can use the `leds` array to update the color of the LEDs. Also, you can use `numLeds()` to get the number of LEDs. You don't need to call `FastLED.show()` here, as it's called automatically by the framework.

For the `getStatus()` method, you can return a string that describes the current state of the animation.

For the `getName()` method, you can return the name of the animation.

For the `init()` method, you can initialize the animation parameters.


## Handling Parameters

Animation parameters allow you to customize how animations behave without modifying their core logic. The `AnimParams` class provides a flexible parameter system using key-value storage with three types:

- Custom floats (`custom_floats`)
- Custom integers (`custom_ints`) 
- Custom color palettes (`custom_palettes`)

Parameters can be accessed using helper methods:
```cpp
// In your animation:
float speed = params.getFloat("speed", 1.0f);      // default: 1.0
int brightness = params.getInt("brightness", 255);  // default: 255
CRGBPalette16 pal = params.getPalette("palette");  // default: Black
```

You can set parameters when creating the animation:
```cpp
animation_manager.add("myanimation", {
    {"speed", "1.5"},          // custom float
    {"brightness", "200"},     // custom int
    {"fade_time", "1000"}     // custom parameter
});
```
