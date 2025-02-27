# PixelTheater Stage Overview

The Stage class is the central component of the PixelTheater library, connecting hardware LEDs, model definitions, and animation scenes. It serves as the primary interface between your application and the visual output, managing the LED data and coordinating scene execution.

## Core Components

The Stage template class takes a ModelDef parameter that defines your LED configuration. It consists of:

- **Platform**: Manages hardware interactions and LED buffer access
- **Model**: Represents the physical arrangement of LEDs based on ModelDef
- **Scenes**: Contains animation logic and visual effects
- **LED Data**: Direct access to the LED color buffer

## Configuration and Setup

### Basic Setup with FastLED

```cpp
#include <FastLED.h>
#include "PixelTheater/platform/fastled_platform.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/stage.h"
#include "models/your_model_def.h"

// Define your LED array
CRGB leds[YourModelDef::LED_COUNT];

void setup() {
    // Configure FastLED with your hardware setup
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, YourModelDef::LED_COUNT);
    // For multiple strips:
    // FastLED.addLeds<WS2812B, PIN1, GRB>(leds, 0, YourModelDef::LED_COUNT/2);
    // FastLED.addLeds<WS2812B, PIN2, GRB>(leds + YourModelDef::LED_COUNT/2, YourModelDef::LED_COUNT/2);
    
    // Set initial FastLED parameters
    FastLED.setBrightness(128);
    FastLED.setMaxRefreshRate(60);
    
    // Create platform with FastLED integration
    auto platform = std::make_unique<FastLEDPlatform>(leds, YourModelDef::LED_COUNT);
    
    // Create model based on your model definition
    auto model = std::make_unique<Model<YourModelDef>>(YourModelDef{}, platform->getLEDs());
    
    // Create stage with platform and model
    auto stage = std::make_unique<Stage<YourModelDef>>(std::move(platform), std::move(model));
    
    // Add and set initial scene
    auto* myScene = stage->addScene<MyScene<YourModelDef>>(*stage);
    stage->setScene(myScene);
}

void loop() {
    // Update stage (calls current scene's tick() and updates LEDs)
    stage->update();  // This will call FastLED.show() internally
}
```

### Custom Model Configuration

For custom LED arrangements, define your own ModelDef:

```cpp
// In your custom model header
struct MyCustomModel {
    static constexpr uint16_t LED_COUNT = 300;
    static constexpr uint8_t FACE_COUNT = 6;
    // Define other model properties
};

// In your application
CRGB leds[MyCustomModel::LED_COUNT];

// Configure FastLED
FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, MyCustomModel::LED_COUNT);

// Create platform with FastLED integration
auto platform = std::make_unique<FastLEDPlatform>(leds, MyCustomModel::LED_COUNT);
auto model = std::make_unique<Model<MyCustomModel>>(MyCustomModel{}, platform->getLEDs());
auto stage = std::make_unique<Stage<MyCustomModel>>(std::move(platform), std::move(model));
```

## LED Access and Manipulation

The Stage provides direct access to LEDs through the `leds` member:

```cpp
// Access individual LEDs
stage->leds[0] = CRGB::Red;

// Bounds checking is built-in (out-of-range indices are clamped to the last valid LED)
stage->leds[9999] = CRGB::Blue;  // Sets the last LED to blue

// Use range-based loops
for (auto& led : stage->leds) {
    led = CRGB::Green;
}

// Apply changes to the hardware
stage->update();  // This will call FastLED.show() internally
```

## Scene Management

Scenes contain the animation logic for your LED display:

```cpp
// Adding a scene (passing the stage reference to the scene)
auto* myScene = stage->addScene<MyCustomScene<ModelDef>>(*stage);

// Switching to a different scene
stage->setScene(myScene);

// Main loop to render current scene
void loop() {
    stage->update();  // Calls current scene's tick() and updates LEDs
}
```

## Scene Implementation

When implementing a scene, you have access to the stage and its components:

```cpp
template<typename ModelDef>
class MyScene : public Scene<ModelDef> {
public:
    using Scene<ModelDef>::Scene;  // Inherit constructor
    
    void setup() override {
        // Initialization code
    }
    
    void tick() override {
        Scene<ModelDef>::tick();  // Call base to increment counter
        
        // Access LEDs through stage
        auto& leds = this->stage.leds;
        leds[0] = CRGB::Red;
        
        // Access model through stage
        auto last_idx = this->stage.model.led_count() - 1;
        leds[last_idx] = CRGB::Blue;
        
        // Access faces through model
        this->stage.model.faces[0].leds[0] = CRGB::Green;
        
        // Use range-based iteration with FastLED functions
        for(auto& led : this->stage.leds) {
            fadeToBlackBy(led, 128);
        }
    }
};
```

## Performance and Hardware Considerations

Our hardware tests demonstrate excellent performance for the library:

- Successfully handles large LED arrays (tested up to 4096 LEDs)
- Memory management is efficient with proper allocation/deallocation
- PixelTheater operations are only approximately 19% slower than native FastLED
- Optimized for microcontroller constraints without exceptions

For applications with limited memory, consider:

- Using smaller LED counts for testing and development
- Monitoring memory usage with built-in tools
- Structuring animations to avoid large temporary allocations

## Error Handling

Since exceptions aren't available on most microcontrollers, the library employs these strategies:

1. Array access is bounds-checked but returns safe values:
   - Out of range indices return the last valid element (clamping behavior)
   - The `leds` member provides safe array access with bounds checking

2. Methods return sentinel values:
   - Invalid geometric queries return zero vectors
   - Size queries return 0 for invalid inputs
   - Color operations silently skip invalid LEDs

This approach prevents crashes while making issues visible during testing.
