#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/core/color.h"
#include <chrono> // For millis()
#include <cstdlib> // For rand()
#include <ctime> // For seeding rand()
#include <cmath> // For fmod
#include <cstdio> // For printf (logging)
#include <cstdarg> // For va_list etc. (logging)

namespace PixelTheater {

// Keep track of the start time for millis()
static const auto start_time = std::chrono::high_resolution_clock::now();
// Seed random number generator once
static bool rand_seeded = false;

NativePlatform::NativePlatform(uint16_t num_leds) 
    : _num_leds(num_leds)
{
    _leds = new CRGB[num_leds]();  // () for zero-initialization
    if (!rand_seeded) {
        srand(static_cast<unsigned int>(time(0)));
        rand_seeded = true;
    }
}

NativePlatform::~NativePlatform() {
    delete[] _leds;
}

CRGB* NativePlatform::getLEDs() {
    return _leds;
}

uint16_t NativePlatform::getNumLEDs() const {
    return _num_leds;
}

void NativePlatform::show() {
    // No-op in native environment
}

void NativePlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

void NativePlatform::clear() {
    fill_solid(_leds, _num_leds, CRGB::Black);
}

void NativePlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
}

void NativePlatform::setDither(uint8_t dither) {
    _dither = dither;
}

// --- Implementations for new Platform virtual methods --- 

float NativePlatform::deltaTime() {
    // Simple fixed delta time for native testing, maybe 1/60th sec?
    // A more sophisticated implementation could track actual time.
    return 1.0f / 60.0f; 
}

uint32_t NativePlatform::millis() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    return static_cast<uint32_t>(duration.count());
}

uint8_t NativePlatform::random8() {
    return static_cast<uint8_t>(rand() & 0xFF);
}

uint16_t NativePlatform::random16() {
    return static_cast<uint16_t>(rand() & 0xFFFF);
}

uint32_t NativePlatform::random(uint32_t max) {
    if (max == 0) return rand();
    // Simple modulo, might have slight bias if max isn't power of 2
    return rand() % max; 
}

uint32_t NativePlatform::random(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    return min + (rand() % (max - min));
}
    
float NativePlatform::randomFloat() { // 0.0 to 1.0
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float NativePlatform::randomFloat(float max) { // 0.0 to max
    return randomFloat() * max;
}

float NativePlatform::randomFloat(float min, float max) { // min to max
    if (min >= max) return min;
    return min + randomFloat() * (max - min);
}

// Logging implementations using vprintf
void NativePlatform::logInfo(const char* format, ...) {
    printf("[INFO] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void NativePlatform::logWarning(const char* format, ...) {
     printf("[WARN] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void NativePlatform::logError(const char* format, ...) {
     printf("[ERROR] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

} // namespace PixelTheater 