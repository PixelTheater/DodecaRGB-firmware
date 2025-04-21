#pragma once

#include "PixelTheater/SceneKit.h" // SceneKit helpers
#include <cmath>       // For std::sin, std::fmod, std::max
#include <algorithm>   // For std::clamp
#include <vector>      // For std::max used with initializer list

namespace Scenes {

class SparklesScene : public Scene {
private:
    // --- Parameters (handled by base class 'settings') ---
    // Settings like "Speed", "Glitter", "Chaos", "Intensity"
    // will be accessed via settings["ParamName"]

    // --- Palettes ---
    CRGBPalette16 palette1; // Will be initialized in setup()
    CRGBPalette16 palette2; // Will be initialized in setup()

    // --- Color State & Transition ---
    CRGB colorA = CRGB::Black;             // Current transitioning color A
    CRGB colorB = CRGB::Black;             // Current transitioning color B

    CRGB colorATarget = CRGB::Black;         // Target color for A's transition
    CRGB colorBTarget = CRGB::Black;         // Target color for B's transition

    CRGB previousColorATarget = CRGB::Black; // Previous target A (start of lerp)
    CRGB previousColorBTarget = CRGB::Black; // Previous target B (start of lerp)

    float colorChangeTimer = 0.0f;        // Countdown (seconds) to *end* of current transitions
    float colorATransitionDuration = 0.0f;// Duration (seconds) for A's current transition
    float colorBTransitionDuration = 0.0f;// Duration (seconds) for B's current transition

    // --- Mix Ratio Oscillation ---
    float mixOscillatorPhase = 0.0f;      // Phase (radians) for the mix calculation

    // --- Helper Methods --- 
    void startNewColorTransition();
    CRGB selectNewColorFromPalette(const CRGBPalette16& palette);
    float calculateBaseTransitionDuration() const;
    float randomizeDuration(float baseDuration);
    CRGB lerpColor(const CRGB& start, const CRGB& end, float factor) const;
    uint8_t calculateFadeAmount() const;
    uint8_t calculateSparkleStrength() const;
    float calculateMixRatio();
    uint8_t calculateSparkleBrightness();

public:
    // Constructor
    SparklesScene() = default;

    // Required lifecycle methods
    void setup() override;
    void tick() override;

    // Optional status method
    std::string status() const override;
};

} // namespace Scenes 