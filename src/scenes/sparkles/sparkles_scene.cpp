#include "sparkles_scene.h"
#include "PixelTheater/SceneKit.h" // Access common scene elements
#include <cmath>                     // For std::sin, std::fmod, std::max, std::round
#include <algorithm>                 // For std::clamp, std::max
#include <vector>                    // For std::max used with initializer list
// #include <string.h> // Not needed for loop-based copy
// #include <avr/pgmspace.h> // Alternative header for PROGMEM utilities if string.h doesn't work
// #include <FastLED.h> // Likely redundant via SceneKit

namespace Scenes {

// --- Constants for Calculations --- 
// Parameter Defaults
static constexpr float DEFAULT_SPEED = 0.3f;
static constexpr float DEFAULT_GLITTER = 0.4f;
static constexpr float DEFAULT_CHAOS = 0.6f;
static constexpr float DEFAULT_INTENSITY = 0.3f;
// Transition Timing
static constexpr float BASE_TRANSITION_MIN_S = 2.0f;
static constexpr float BASE_TRANSITION_SPEED_SCALE = 8.0f;
static constexpr float MIN_RANDOMIZED_DURATION_S = 0.1f;
static constexpr float CHAOS_DURATION_FACTOR = 0.5f;
static constexpr float INITIAL_FADE_IN_DURATION_S = 1.0f;
// Fading
static constexpr uint8_t BASE_FADE = 3;
static constexpr float FADE_INTENSITY_SCALE = 15.0f;
// Sparkle Strength & Density
static constexpr uint8_t BASE_SPARKLE_STRENGTH = 64;
static constexpr float SPARKLE_INTENSITY_SCALE = 191.0f; // 255.0f - 64.0f
static constexpr float SPARKLE_DENSITY_FACTOR = 0.5f;
// Mix Ratio Oscillation
static constexpr float BASE_MIX_FREQ = 0.3f;
static constexpr float MIX_FREQ_SPEED_SCALE = 0.05f;
static constexpr float CHAOS_MIX_FREQ_FACTOR = 0.5f;
static constexpr float GLITTER_MIX_AMPLITUDE_SCALE = 0.5f;
static constexpr float CHAOS_MIX_NOISE_SCALE = 0.1f;
// Sparkle Brightness Variation
static constexpr float BRIGHTNESS_TIME_SCALE = 10000.0f;
static constexpr float BRIGHTNESS_FREQ = 0.5f;
static constexpr uint8_t BASE_SPARKLE_BRIGHTNESS = 128;
static constexpr float GLITTER_BRIGHTNESS_SCALE = 254.0f;


// === Helper Implementations ===

void SparklesScene::startNewColorTransition() {
    // Store current targets as previous
    previousColorATarget = colorATarget;
    previousColorBTarget = colorBTarget;

    // Choose new colorATarget, colorBTarget (using selectNewColorFromPalette)
    colorATarget = selectNewColorFromPalette(palette1);
    colorBTarget = selectNewColorFromPalette(palette2);

    // Calculate new colorATransitionDuration, colorBTransitionDuration
    float baseDuration = calculateBaseTransitionDuration();
    colorATransitionDuration = randomizeDuration(baseDuration);
    colorBTransitionDuration = randomizeDuration(baseDuration);

    // Reset colorChangeTimer
    colorChangeTimer = std::max(colorATransitionDuration, colorBTransitionDuration);
}

CRGB SparklesScene::selectNewColorFromPalette(const CRGBPalette16& pal) {
    // Use member random8()
    return colorFromPalette(pal, random8());
}

float SparklesScene::calculateBaseTransitionDuration() const {
    float speed = settings["Speed"];
    // Consider using a constant for the base range (e.g., 2.0f, 8.0f)
    return BASE_TRANSITION_MIN_S + (1.0f - speed) * BASE_TRANSITION_SPEED_SCALE;
}

float SparklesScene::randomizeDuration(float baseDuration) {
    float chaos = settings["Chaos"];
    float factor = randomFloat(-chaos * CHAOS_DURATION_FACTOR, chaos * CHAOS_DURATION_FACTOR);
    // Consider using a constant for the minimum duration (0.1f)
    return std::max(MIN_RANDOMIZED_DURATION_S, baseDuration * (1.0f + factor));
}

CRGB SparklesScene::lerpColor(const CRGB& start, const CRGB& end, float factor) const {
    uint8_t t = static_cast<uint8_t>(std::round(factor * 255.0f));
    // Use unqualified lerp8by8 now available via SceneKit
    return CRGB(
        lerp8by8(start.r, end.r, t),
        lerp8by8(start.g, end.g, t),
        lerp8by8(start.b, end.b, t)
    );
}

uint8_t SparklesScene::calculateFadeAmount() const {
    float intensity = settings["Intensity"];
    float glitter = settings["Glitter"];
    // Fade amount increases with intensity and glitter
    float combinedFactor = (intensity + glitter) / 2.0f;
    uint8_t fadeAmount = BASE_FADE + static_cast<uint8_t>(combinedFactor * FADE_INTENSITY_SCALE);
    // Ensure fadeAmount doesn't exceed a reasonable max (e.g., 255 is full fade, maybe cap lower?)
    // Let's cap it slightly below max to avoid instant blackouts unless desired.
    return std::min(fadeAmount, (uint8_t)250);
}

uint8_t SparklesScene::calculateSparkleStrength() const {
    float intensity = settings["Intensity"];
    // Consider using constants for the strength range (64, 255-64)
    return BASE_SPARKLE_STRENGTH + static_cast<uint8_t>(intensity * SPARKLE_INTENSITY_SCALE);
}

float SparklesScene::calculateMixRatio() {
    float speed = settings["Speed"];
    float chaos = settings["Chaos"];
    float glitter = settings["Glitter"];

    // Update phase (example frequency calculation)
    // Consider constants for frequency calculation (0.1f, 0.4f)
    float freq = BASE_MIX_FREQ + speed * MIX_FREQ_SPEED_SCALE;
    float chaosFactor = 1.0f + randomFloat(-chaos * CHAOS_MIX_FREQ_FACTOR, chaos * CHAOS_MIX_FREQ_FACTOR);
    mixOscillatorPhase = std::fmod(mixOscillatorPhase + freq * chaosFactor * deltaTime(), PT_TWO_PI);

    float baseMix = 0.5f + 0.5f * std::sin(mixOscillatorPhase);

    // Apply glitter influence (amplitude) and chaos (small noise)
    float glitterAmount = glitter * GLITTER_MIX_AMPLITUDE_SCALE;
    // Consider constant for chaos noise scaling (0.1f)
    float chaosNoise = randomFloat(-chaos * CHAOS_MIX_NOISE_SCALE, chaos * CHAOS_MIX_NOISE_SCALE);

    float mix = baseMix + chaosNoise;
    float range = 0.5f + glitterAmount;
    mix = (mix - 0.5f) * (range / 0.5f) + 0.5f; // Scale to the glitter range

    return std::clamp(mix, 0.0f, 1.0f);
}

uint8_t SparklesScene::calculateSparkleBrightness() {
    float glitter = settings["Glitter"];
    float time = millis() / BRIGHTNESS_TIME_SCALE;
    // Consider constant for sine wave frequency (5.0f)
    float brightnessFactor = 0.5f + 0.5f * std::sin(time * BRIGHTNESS_FREQ + mixOscillatorPhase);
    // Consider constant for brightness scaling range (128, 254)
    return BASE_SPARKLE_BRIGHTNESS + static_cast<uint8_t>(glitter * (brightnessFactor - 0.5f) * GLITTER_BRIGHTNESS_SCALE);
}

// === Lifecycle Methods ===

void SparklesScene::setup() {
    // --- Define Metadata ---
    set_name("Sparkles");
    set_author("Somebox");
    set_description("Shimmering sparkles with transitioning colors.");
    set_version("2.1");

    // --- Define Parameters (using static constexpr defaults) ---
    param("Speed", "ratio", DEFAULT_SPEED, "clamp", "Color transition frequency and mix oscillation rate");
    param("Glitter", "ratio", DEFAULT_GLITTER, "clamp", "Sparkle brightness variation and mix amplitude");
    param("Chaos", "ratio", DEFAULT_CHAOS, "clamp", "Randomness in timing and mix oscillation");
    param("Intensity", "ratio", DEFAULT_INTENSITY, "clamp", "Overall sparkle density and fade rate");

    // --- Initialize Palettes ---
    // Use PixelTheater platform-independent palettes
    for (int i = 0; i < 16; ++i) {
        // Assuming PixelTheater::Palettes::CloudColors is a CRGBPalette16
        palette1[i] = PixelTheater::Palettes::CloudColors[i];
        palette2[i] = PixelTheater::Palettes::HeatColors[i];
    }

    // --- Initialize State ---
    colorA = CRGB::Black;
    colorB = CRGB::Black;
    previousColorATarget = CRGB::Black;
    previousColorBTarget = CRGB::Black;

    colorATarget = selectNewColorFromPalette(palette1);
    colorBTarget = selectNewColorFromPalette(palette2);

    float baseDuration = calculateBaseTransitionDuration();
    colorATransitionDuration = randomizeDuration(baseDuration);
    colorBTransitionDuration = randomizeDuration(baseDuration);

    colorChangeTimer = std::max({INITIAL_FADE_IN_DURATION_S, colorATransitionDuration, colorBTransitionDuration}); // Initial 3s fade-in

    mixOscillatorPhase = randomFloat(0.0f, PT_TWO_PI);

    logInfo("SparklesScene setup complete"); // Added log message
}

void SparklesScene::tick() {
    float dt = deltaTime();

    // 1. Global Fade (Per-LED method)
    uint8_t fade = calculateFadeAmount();
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        leds[i].fadeToBlackBy(fade); // Use CRGB method via leds proxy
    }

    // 2. Color Transition Timing
    colorChangeTimer -= dt;
    if (colorChangeTimer <= 0) {
        startNewColorTransition();
    }

    // 3. Color Transition Calculation
    float timeRemaining = std::max(0.0f, colorChangeTimer);
    float progressA = (colorATransitionDuration > 0.001f) ? 1.0f - std::clamp(timeRemaining / colorATransitionDuration, 0.0f, 1.0f) : 1.0f;
    float progressB = (colorBTransitionDuration > 0.001f) ? 1.0f - std::clamp(timeRemaining / colorBTransitionDuration, 0.0f, 1.0f) : 1.0f;

    colorA = lerpColor(previousColorATarget, colorATarget, progressA);
    colorB = lerpColor(previousColorBTarget, colorBTarget, progressB);

    // 4. Calculate Mix Ratio
    float mixRatio = calculateMixRatio();

    // 5. Pixel Sparkles
    float intensity = settings["Intensity"];
    // Consider constant for sparkle density factor (0.1f)
    uint16_t numTotalSparkles = static_cast<uint16_t>(intensity * intensity * ledCount() * SPARKLE_DENSITY_FACTOR);
    uint8_t sparkleStrength = calculateSparkleStrength();

    uint16_t numSparklesA = static_cast<uint16_t>(std::round(numTotalSparkles * mixRatio));
    uint16_t numSparklesB = static_cast<uint16_t>(std::round(numTotalSparkles * (1.0f - mixRatio)));

    // Add Sparkles A
    for (uint16_t i = 0; i < numSparklesA; ++i) {
        uint16_t px = random16() % ledCount();
        uint8_t brightnessVariation = calculateSparkleBrightness();
        CRGB variedColorA = colorA;
        variedColorA.nscale8(brightnessVariation); // Apply brightness variation first
        leds[px] += variedColorA.nscale8(sparkleStrength); // Add scaled color
    }

    // Add Sparkles B
    for (uint16_t i = 0; i < numSparklesB; ++i) {
        uint16_t px = random16() % ledCount();
        uint8_t brightnessVariation = calculateSparkleBrightness();
        CRGB variedColorB = colorB;
        variedColorB.nscale8(brightnessVariation); // Apply brightness variation first
        leds[px] += variedColorB.nscale8(sparkleStrength); // Add scaled color
    }
}

std::string SparklesScene::status() const {
    char buffer[128];
     snprintf(buffer, sizeof(buffer), "A:(%d,%d,%d) B:(%d,%d,%d) T:%.2f",
             colorA.r, colorA.g, colorA.b,
             colorB.r, colorB.g, colorB.b,
             colorChangeTimer);
    return std::string(buffer);
}

} // namespace Scenes