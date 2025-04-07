#pragma once

#include <vector>
#include <memory> // For std::unique_ptr, std::move
#include <cstddef> // For size_t
#include <stdexcept> // For exceptions
#include <utility> // For std::move
#include <cassert> // For assert

// Include full INTERFACE definitions needed by Theater members/methods
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/core/iled_buffer.h"
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/scene.h" // Uses interfaces

// Include full WRAPPER & CONCRETE definitions needed for template implementations
#include "PixelTheater/platform/native_platform.h"
#ifdef PLATFORM_TEENSY // Only include FastLED platform specifics for Teensy builds
#include "PixelTheater/platform/fastled_platform.h"
#endif
#include "PixelTheater/model/model.h"
#include "PixelTheater/model_def.h"
#include "PixelTheater/core/model_wrapper.h" // Include wrapper header
#include "PixelTheater/core/led_buffer_wrapper.h" // Include wrapper header

// Forward declarations
namespace PixelTheater {
    class Scene;
    class Platform;
    class IModel;
    class ILedBuffer;
    // Forward declare specific platform/model if needed for templates
    class NativePlatform;
#ifdef PLATFORM_TEENSY // Only forward declare FastLED platform for Teensy builds
    class FastLEDPlatform; 
#endif
    template<typename TModelDef> class Model;
    template<typename TModelDef> class ModelWrapper;
    class LedBufferWrapper;
    struct ModelDefinitionBase; // Base for ModelDef concept (if needed)
} 

// Forward declare testing namespace
// namespace TheaterTesting {} // No longer needed

namespace PixelTheater {

// --- REMOVE Inline Wrapper Definitions --- 
// class LedBufferWrapper : public ILedBuffer { ... };
// template<typename TModelDef> class ModelWrapper : public IModel { ... };

/**
 * @brief Main facade class for the PixelTheater library.
 * 
 * Manages the platform, model, LED buffer, and scene execution.
 * Users interact primarily with this class to initialize the system,
 * add scenes, and control playback.
 */
class Theater {
    // friend namespace TheaterTesting; // Invalid syntax, removed

public:
    Theater();
    ~Theater();

    // Prevent copying/moving for simplicity for now
    // (Could add move semantics later if needed)
    Theater(const Theater&) = delete;
    Theater& operator=(const Theater&) = delete;
    Theater(Theater&&) = delete;
    Theater& operator=(Theater&&) = delete;

    // --- Initialization (To be implemented in Task 8) ---
    // template<typename TModelDef, typename TPlatform> 
    // void usePlatform(Args... args); // Generic idea
    // e.g.: template<typename TModelDef> void useFastLEDPlatform(...);
    /**
     * @brief Initialize the Theater to use the NativePlatform for testing.
     * 
     * @tparam TModelDef The specific ModelDefinition struct for the geometry.
     * @param num_leds The total number of LEDs (used by NativePlatform).
     */
    template<typename TModelDef>
    void useNativePlatform(size_t num_leds);

#ifdef PLATFORM_TEENSY // Only declare useFastLEDPlatform for Teensy builds
    /**
     * @brief Initialize the Theater to use the FastLEDPlatform.
     * 
     * @tparam TModelDef The specific ModelDefinition struct for the geometry.
     * @param leds Pointer to the FastLED CRGB array (::CRGB*).
     * @param num_leds The total number of LEDs in the array.
     */
    template<typename TModelDef>
    void useFastLEDPlatform(::CRGB* leds, size_t num_leds);
#endif

    // --- Scene Management (Task 9) ---
    template<typename SceneType>
    void addScene();

    void start(); 
    void nextScene(); 
    void previousScene(); 
    void update(); 
    
    // --- Scene Access (Task 9) ---
    Scene& scene(size_t index);
    const Scene& scene(size_t index) const;
    const std::vector<std::unique_ptr<Scene>>& scenes() const;
    Scene* currentScene();
    const Scene* currentScene() const;
    size_t sceneCount() const;

protected:
    // Core components managed by the Theater
    std::unique_ptr<Platform> platform_;
    std::unique_ptr<IModel> model_;      // Holds the ModelWrapper
    std::unique_ptr<ILedBuffer> leds_;   // Holds the LedBufferWrapper
    
    std::vector<std::unique_ptr<Scene>> scenes_;
    Scene* current_scene_ = nullptr;

    // Internal state flag
    bool initialized_ = false;

    // --- Internal Methods (Task 8) ---
    
    /**
     * @brief Internal helper to set up platform, model, and LED buffer wrappers.
     *
     * Takes ownership of the provided platform instance.
     * Creates the necessary wrappers and concrete model instance.
     * 
     * @tparam TModelDef The model definition.
     * @tparam TPlatform The concrete platform type.
     * @param platform A unique_ptr to the platform instance.
     */
    template<typename TModelDef, typename TPlatform>
    void internal_prepare(std::unique_ptr<TPlatform> platform);

private:
    // No private members needed currently?
    // Add any truly private implementation details here if necessary.

};

// --- Template Method Implementations --- 
// (Rely on full includes above)

template<typename TModelDef>
void Theater::useNativePlatform(size_t num_leds) {
    if (initialized_) {
        // Removed throw, just return
        return; 
    }
    auto platform = std::make_unique<NativePlatform>(num_leds);
    internal_prepare<TModelDef, NativePlatform>(std::move(platform));
}

#ifdef PLATFORM_TEENSY // Only define useFastLEDPlatform for Teensy builds
template<typename TModelDef>
void Theater::useFastLEDPlatform(::CRGB* leds, size_t num_leds) {
     if (initialized_) {
        // Removed throw, just return
        return; 
    }
    auto platform_leds = reinterpret_cast<PixelTheater::CRGB*>(leds);
    auto platform = std::make_unique<FastLEDPlatform>(platform_leds, num_leds);
    internal_prepare<TModelDef, FastLEDPlatform>(std::move(platform));
}
#endif

template<typename TModelDef, typename TPlatform>
void Theater::internal_prepare(std::unique_ptr<TPlatform> platform) {
    if (!platform) {
        // Cannot log, just return
         return; 
    }
    platform_ = std::move(platform);
    TModelDef model_def_instance;
    auto concrete_model = std::make_unique<Model<TModelDef>>(
        model_def_instance, 
        platform_->getLEDs() 
    );
    model_ = std::make_unique<ModelWrapper<TModelDef>>(std::move(concrete_model));
    leds_ = std::make_unique<LedBufferWrapper>(
        platform_->getLEDs(), 
        platform_->getNumLEDs()
    );
    initialized_ = true;
    if (platform_) platform_->logInfo("Theater initialized.");
}

template<typename SceneType>
void Theater::addScene() {
    if (!initialized_) {
        // Cannot log, just return
        return; 
    }
    static_assert(std::is_base_of<Scene, SceneType>::value, "SceneType must inherit from PixelTheater::Scene");
    auto new_scene = std::make_unique<SceneType>();
    Scene* scene_ptr = new_scene.get();
    scene_ptr->connect(*model_, *leds_, *platform_); 
    scenes_.push_back(std::move(new_scene));
    if (!current_scene_) {
        current_scene_ = scene_ptr;
    }
}

} // namespace PixelTheater 