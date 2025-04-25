#pragma once

#include "PixelTheater/SceneKit.h" // SceneKit helpers
#include <cmath>       // For std::sin, std::fmod, std::max
#include <algorithm>   // For std::clamp
#include <vector>      // For std::max used with initializer list

namespace Scenes {

class SparklesScene : public Scene {
public:
    // === Parameter Defaults ===
    static constexpr float DEFAULT_SPEED = 0.4f;
    static constexpr float DEFAULT_GLITTER = 0.5f;
    static constexpr float DEFAULT_CHAOS = 0.4f;
    static constexpr float DEFAULT_INTENSITY = 0.5f;

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

    // --- Mix Ratio State ---
    float mixOscillatorPhase = 0.0f;      
    float currentMixRatio_ = 0.5f;        // Current actual mix ratio (lerped)
    float targetMixRatio_ = 0.5f;         // Target mix ratio (changes abruptly)
    float mixOscillationFreq_ = 0.1f;     // Current actual frequency (lerped)
    float targetMixOscillationFreq_ = 0.1f; // Target frequency (changes based on Speed/Chaos)

    // --- Evolving Chaos State ---
    float currentChaosLevel_ = 0.0f;      // Current effective chaos (lerped)
    float targetChaosLevel_ = 0.0f;       // Target chaos level (changes periodically)
    // Timer for chaos target change can reuse colorChangeTimer

    // --- Initial Transition Flag ---
    bool is_initial_transition = true;    

    // === Helper Method Declarations ===
    void startNewColorTransition(float speed, float chaosParam);
    CRGB selectNewColorFromPalette(const CRGBPalette16& pal);
    float calculateBaseTransitionDuration(float speed) const;
    float randomizeDuration(float baseDuration, float chaos);
    CRGB lerpColor(const CRGB& start, const CRGB& end, float factor) const;
    uint8_t calculateFadeAmount(float intensity, float glitter) const;
    uint8_t calculateSparkleStrength(float intensity) const;
    uint8_t calculateSparkleBrightness(float glitter);

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