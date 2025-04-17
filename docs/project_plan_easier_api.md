# PixelTheater Easier API Refactoring - Project Plan

## Palette Restructuring (Platform Transparent API)

### Goal Summary

To provide a simple, reliable, and **platform-transparent** way for scenes to use color palettes and utilities. Scenes will interact solely with the `PixelTheater` namespace, which will internally handle platform differences (FastLED on Teensy, C++ emulation/fallbacks elsewhere) without requiring conditional compilation within scene code.

### Core Principles

1.  **Consistent `PixelTheater` Namespace**: All types (`CRGB`, `CHSV`, `CRGBPalette16`) and core functions (`colorFromPalette`, `blend`, etc.) used by scenes reside within and are accessed via the `PixelTheater` namespace.
2.  **Internal Platform Handling**: The *implementation* of `PixelTheater` types and functions will conditionally use FastLED natives (when `PLATFORM_TEENSY`) or C++ fallback/emulation logic.
3.  **API Abstraction**: Scenes include only `PixelTheater` headers, insulating them from underlying FastLED details or fallback implementations.
4.  **Unified Palette Access**: All standard FastLED palettes and project-specific custom palettes will be exposed as `constexpr` or `extern const` constants within the `PixelTheater::Palettes` namespace (e.g., `PixelTheater::Palettes::Rainbow`, `PixelTheater::Palettes::basePalette`).

### Essential Information

Native tests are run with: `~/.platformio/penv/bin/pio test -e native`
Teensy41 build is run with: `~/.platformio/penv/bin/pio run -e teensy41`
Web build can be checked with `./build_web.sh`
The native tests do not test `/src` or the scenes. To test these, we must run the teensy41 build.
You can run `git diff` to see what's been changed since the tests were last passing.
Python tests can be run with `python -m util.tests.run_tests`

### Implementation Plan

1.  **Solidify Core Types (`core/crgb.h`, `core/crgb.cpp`)**:
    *   **Action**: Ensure `PixelTheater::CRGB` and `PixelTheater::CHSV` class definitions in `core/crgb.h` serve all platforms.
    *   **Action**: Define static color constants (`PixelTheater::CRGB::Red`, etc.) in `core/crgb.cpp`.
    *   **Action (Teensy Opt.)**: Add conversion operators (`operator ::CRGB() const`, `PixelTheater::CRGB(const ::CRGB&)`) within `PixelTheater::CRGB` guarded by `#ifdef PLATFORM_TEENSY` to facilitate efficient calls to underlying FastLED functions.
    *   **Testing/Verification**: Build succeeds for native and Teensy targets. Basic instantiation tests pass.

2.  **Define Palette Type & Constants (`palettes.h`, `palettes.cpp`)**:
    *   **Action**: Create/Update `lib/PixelTheater/include/PixelTheater/palettes.h`.
        *   Include `PixelTheater/core/crgb.h`.
        *   Define `using CRGBPalette16 = std::array<PixelTheater::CRGB, 16>;` (platform-independent).
        *   Declare *all* required palettes (standard + custom) as `extern const PixelTheater::CRGBPalette16 PixelTheater::Palettes::XYZ;`. Use a nested `Palettes` namespace for organization.
    *   **Action**: Create `lib/PixelTheater/src/palettes.cpp`.
        *   Include `palettes.h`.
        *   Define the actual palette arrays using `PixelTheater::CRGB` values (copying FastLED source values for standard palettes like `Rainbow`, `Party`, etc., and defining custom ones like `basePalette`).
    *   **Testing/Verification**: Build succeeds. Native tests can access `PixelTheater::Palettes::basePalette` etc. Teensy build confirms standard FastLED palettes (e.g., `PartyColors_p`) are accessible globally *if* `palettes.h` includes `<FastLED.h>` under `#ifdef PLATFORM_TEENSY` (Confirm this include is needed/desired here or only where used).

3.  **Implement Abstracted API (`color_api.h`/`.cpp`, `core/crgb.h`/`.cpp`)**:
    *   **Action**: Define platform-independent function signatures in a suitable header (e.g., `color_api.h` or `color_utils.h`). Key functions:
        *   `PixelTheater::CRGB PixelTheater::colorFromPalette(const PixelTheater::CRGBPalette16& pal, uint8_t index, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND);` (Define `PixelTheater::TBlendType` enum: `LINEARBLEND`, `NOBLEND`).
        *   `PixelTheater::CRGB PixelTheater::blend(const PixelTheater::CRGB& p1, const PixelTheater::CRGB& p2, uint8_t amount);`
        *   (Add others as needed, like `nblend`).
    *   **Action**: Add core methods to `PixelTheater::CRGB` in `core/crgb.h`:
        *   `CRGB& fadeToBlackBy(uint8_t fade);`
        *   `CRGB& nscale8(uint8_t scale);`
        *   (Add operators like `+=`, `-=`, `*=`, etc. if not already present).
    *   **Action**: Implement these functions and methods in their respective `.cpp` files.
        *   Use `#ifdef PLATFORM_TEENSY` to call the corresponding FastLED global function or `::CRGB` method (using conversion operators if needed).
        *   In the `#else` block, implement the logic using standard C++ (e.g., manual interpolation for `colorFromPalette`, component-wise math for `blend`, `fade`, `scale`).
    *   **Action**: Fixed template `fill_solid` implementation in `core/color.h` to work with `LedsProxy`.
    *   **Testing/Verification**: Native tests verify C++ fallback implementations (blending math, scaling, fading, palette lookup). Teensy build checks confirm API calls delegate to FastLED correctly (may require inspection or specific Teensy tests if possible). Build succeeds.
    *   **Status**: COMPLETE

4.  **Refactor Color Utilities (`color_utils.h`/`.cpp`)**: (IN PROGRESS)
    *   **Action**: Ensure all functions use only `PixelTheater` types (`CRGB`, `CHSV`, `std::string`).
    *   **Action**: Remove any internal platform-specific stubs (like the fallback `rgb2hsv_approximate` or `get_perceived_brightness`) as the type/API abstraction now handles platform differences. Ensure `color_utils.cpp` includes the necessary headers to use the abstracted API (e.g., for `rgb2hsv`).
    *   **Testing/Verification**: Existing native tests in `test_color_utils.cpp` should still pass using the abstracted API underlyingly.

5.  **Update Python Generator (`util/generate_props.py`)**: (COMPLETE)
    *   **Action**: Update validation logic (2-16 entries, index rules). (COMPLETE)
    *   **Action**: Output C++ code defining `constexpr PixelTheater::GradientPaletteData { const uint8_t* data; size_t size; };` structs into `extra_palettes.h` (filename changed from `generated_palettes.h`). (COMPLETE)
    *   **Action**: Moved palette JSON source files to `util/palettes/`. (COMPLETE)
    *   **Action**: Added new palettes (Party, Rainbow, Forest). (COMPLETE)
    *   **Testing/Verification**: Python unit tests (`test_generate_props.py`) pass for validation and output format. Generated header compiles. (COMPLETE)
    *   **Note**: Using these generated gradients will require platform-specific code (`#ifdef PLATFORM_TEENSY`) in scenes for now, as the native `colorFromPalette` won't support them initially.

6.  **Remove Deprecated Palette Param Type**: (COMPLETE)
    *   **Action**: Clean up `ParamType::palette` and related logic from the parameter system files.
    *   **Testing/Verification**: Build succeeds. Parameter system tests (if any) pass.
    *   **Status**: COMPLETE

7.  **Refactor Active Scenes (`src/scenes/`)**: (IN PROGRESS)
    *   **Action**: Update includes (`color_api.h`, `palettes.h`, etc.).
    *   **Action**: Ensure all color/palette/math operations use the `PixelTheater` namespace and abstracted API/methods.
    *   **Testing/Verification**: Scenes compile and run correctly in native (using fallbacks) and Teensy (using FastLED) environments.
    *   **Refactored**: `blob_scene.h`, `xyz_scanner.h`, `wandering_particles.h`, `test_scene.h`, `boids_scene.h`

8.  **Documentation (`docs/PixelTheater/Palettes.md`, examples)**:
    *   **Action**: Rewrite documentation to reflect the platform-transparent `PixelTheater` API.
    *   **Action**: Emphasize using `PixelTheater::Palettes::XYZ`, `PixelTheater::colorFromPalette`, `PixelTheater::blend`, `PixelTheater::CRGB::fadeToBlackBy`, etc.
    *   **Action**: Remove platform conditional compilation from examples. Explain the Python generator utility and the current limitation of generated gradients for non-Teensy platforms.
    *   **Testing/Verification**: Documentation accurately reflects the final API and usage patterns.

### Scene Usage Pattern Examples (Revised for Abstraction)

```cpp
// Include necessary PixelTheater headers
#include "PixelTheater/palettes.h"   // Provides PixelTheater::Palettes::* constants & CRGBPalette16 type
#include "PixelTheater/color_api.h"  // Provides PixelTheater::colorFromPalette, blend, etc.
#include "PixelTheater/color_utils.h" // Provides PixelTheater::ColorUtils::* helpers

// Within your scene class (no #ifdefs needed for platform)

void MyScene::tick() {
    uint8_t brightness = 255;
    uint8_t index1 = millis() / 20; 
    uint8_t index2 = millis() / 30;

    // --- Using Palettes (Always via PixelTheater namespace) ---

    // 1. Using a standard palette (e.g., PartyColors from FastLED)
    PixelTheater::CRGB color1 = PixelTheater::colorFromPalette(
        PixelTheater::Palettes::Party, // Constant defined in palettes.cpp
        index1, brightness, PixelTheater::LINEARBLEND);
    leds[0] = color1;

    // 2. Using a custom built-in palette 
    PixelTheater::CRGB color2 = PixelTheater::colorFromPalette(
        PixelTheater::Palettes::basePalette, // Constant defined in palettes.cpp
        index2, brightness, PixelTheater::LINEARBLEND);
    leds[1] = color2;

    // --- Using Color Utilities ---
    PixelTheater::CRGB currentColor = leds[1];
    PixelTheater::CHSV currentHSV = PixelTheater::ColorUtils::rgb2hsv_approximate(currentColor); 
    std::string colorName = PixelTheater::ColorUtils::getClosestColorName(currentColor);
    // Serial.printf("LED 1 (%s) - HSV: %d,%d,%d\n", colorName.c_str(), currentHSV.h, currentHSV.s, currentHSV.v);

    // --- Blending/Modifying Colors (Using PixelTheater API/Methods) ---

    // Fade all LEDs slightly
    for(int i=0; i<numLeds(); ++i) {
        leds[i].fadeToBlackBy(10); // Uses PixelTheater::CRGB::fadeToBlackBy
    }
    
    // Blend LED 0 towards white
    leds[0] = PixelTheater::blend(leds[0], PixelTheater::CRGB::White, 32); // Uses PixelTheater::blend
}
```

### Pending Decisions / Future Considerations

-   **Native Gradient Support**: Implementing `PixelTheater::colorFromPalette` for gradient data (`uint8_t[]`) in the native C++ fallback is deferred.
-   **Optional Name Lookup**: Parameter handling via name lookup (`param("palette_name", ...)`) remains deferred.

### Non-Goals

-   Runtime palette loading, modification, or complex management in C++.
-   Automatic C++ lookup generation by the Python script.

### Migration Plan

1.  Update existing scenes to use the new `param("palette", ...)` API.
2.  Deprecate and eventually remove any old palette access methods.
3.  Ensure all documentation is updated.
4.  Provide examples or a brief migration guide if changes are significant for scene developers.

### Success Metrics

1.  **Technical**: All Python and C++ tests pass; validation rules are consistent; build system generates correct code; documentation is accurate.
2.  **User Experience**: Scene developers find it easier and more reliable to configure and use palettes; error messages are clear; behavior is predictable.

### API Examples

```cpp
// Scene palette usage
void config() override {
    param("palette", Palette, "ocean");  // Simple palette reference
}

void tick() override {
    // Easy FastLED conversion
    auto colors = palette().to_fastled();
    
    // Direct color access
    CRGB color = palette().color_at(position);
}
```

### Validation Rules

1.  **Palette File Format**
    *   Must contain 'name' and 'palette' fields
    *   Palette must have 2-16 entries (8-64 bytes)
    *   Each entry must have index, R, G, B values (0-255)
    *   Indices must be ascending (0 to 255)
    *   First index must be 0, last must be 255

2.  **Runtime Validation**
    *   Palette data must match format requirements
    *   Names must be unique within scope
    *   Scene palette references must exist
    *   FastLED conversion must preserve color accuracy

### Future Considerations

1.  **Potential Enhancements**
    *   CSS3 gradient support
    *   Runtime palette modification
    *   Palette interpolation methods
    *   Palette animation support

2.  **Performance Optimization**
    *   Evaluate memory usage
    *   Consider caching strategies
    *   Optimize FastLED conversion

