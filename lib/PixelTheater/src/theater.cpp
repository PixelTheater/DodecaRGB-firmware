#include "PixelTheater/theater.h"

// Includes needed only for non-template method implementations
#include "PixelTheater/scene.h" 
#include "PixelTheater/core/log.h" // Needed for logging
// #include "PixelTheater/platform/platform.h"
// #include "PixelTheater/core/imodel.h"
// #include "PixelTheater/core/iled_buffer.h"
// #include "PixelTheater/platform/native_platform.h" 
// #include "PixelTheater/model/model.h" 
// #include "PixelTheater/model_def.h" 
// #include "PixelTheater/core/model_wrapper.h" 
// #include "PixelTheater/core/led_buffer_wrapper.h" 
// #include <stdexcept> 

namespace PixelTheater {

// Define a dummy scene class locally for error returns
class DummyScene : public Scene {
public:
    DummyScene() { set_name("DummyScene"); }
    void setup() override {}
    void tick() override {}
};
// Static instance to return reference to
static DummyScene dummy_scene_instance;

Theater::Theater() {
    // Constructor implementation (currently empty)
}

Theater::~Theater() {
    // Destructor implementation
    // unique_ptrs will handle cleanup automatically
}

// --- Template Implementations moved to theater.h --- 

// --- Non-template implementations for Theater methods --- 

void Theater::start() {
    if (!initialized_) { 
        // Cannot log reliably before platform is set.
        // printf("[ERROR] Theater::start() called before initialization.\n");
        return; 
    }
    if (!current_scene_) {
        if (!scenes_.empty()) {
            current_scene_ = scenes_[0].get();
        } else {
            if (platform_) platform_->logWarning("Theater::start() called with no scenes added.");
            return; 
        }
    }
    // TODO: Add setup_called flag to Scene?
    if (platform_) platform_->logInfo("Theater started.");
    current_scene_->setup(); 
}

void Theater::update() {
    if (!initialized_ || !current_scene_ || !platform_) return; // Nothing to do
    
    current_scene_->tick();
    platform_->show();
}

void Theater::nextScene() {
    if (scenes_.size() < 2) return; 
    
    size_t current_index = scenes_.size(); // Initialize to invalid index
    for(size_t i = 0; i < scenes_.size(); ++i) {
        if (scenes_[i].get() == current_scene_) {
            current_index = i;
            break;
        }
    }

    if (current_index < scenes_.size()) { // Check if found
        size_t next_index = (current_index + 1) % scenes_.size();
        current_scene_ = scenes_[next_index].get();
        current_scene_->reset(); 
        current_scene_->setup(); 
    } else if (!scenes_.empty()) {
        current_scene_ = scenes_[0].get();
        current_scene_->reset();
        current_scene_->setup();
    }
}

void Theater::previousScene() {
     if (scenes_.size() < 2) return;
    
    size_t current_index = scenes_.size(); // Initialize to invalid index
    for(size_t i = 0; i < scenes_.size(); ++i) {
        if (scenes_[i].get() == current_scene_) {
            current_index = i;
            break;
        }
    }

    if (current_index < scenes_.size()) { // Check if found
        size_t prev_index = (current_index == 0) ? (scenes_.size() - 1) : (current_index - 1);
        current_scene_ = scenes_[prev_index].get();
        current_scene_->reset(); 
        current_scene_->setup();
    } else if (!scenes_.empty()) {
        current_scene_ = scenes_[0].get();
        current_scene_->reset();
        current_scene_->setup();
    }
}

// --- Accessors ---

size_t Theater::sceneCount() const {
    return scenes_.size();
}

Scene* Theater::currentScene() {
    return current_scene_;
}

const Scene* Theater::currentScene() const {
    return current_scene_;
}

const std::vector<std::unique_ptr<Scene>>& Theater::scenes() const {
    return scenes_;
}

Scene& Theater::scene(size_t index) {
    if (index >= scenes_.size()) {
        if (platform_) platform_->logError("Theater::scene index out of range");
        // Return dummy reference for graceful failure
        return dummy_scene_instance; 
    }
    return *scenes_[index];
}

const Scene& Theater::scene(size_t index) const {
     if (index >= scenes_.size()) {
        if (platform_) platform_->logError("Theater::scene const index out of range"); 
        // Return dummy reference for graceful failure
        return dummy_scene_instance; 
    }
    return *scenes_[index];
}

// --- ADDED: Platform Accessor ---
Platform* Theater::platform() {
    return platform_.get();
}

const Platform* Theater::platform() const {
    return platform_.get();
}

// --- ADDED: Scene Control ---
bool Theater::setScene(size_t index) {
    if (!initialized_) {
        // Log::error should ideally be used, but platform might not be available.
        printf("[ERROR] Theater::setScene called before initialization.\n");
        return false;
    }
    if (index >= scenes_.size()) {
        if (platform_) platform_->logWarning("Theater::setScene index out of range: %zu", index);
        return false;
    }

    // Get the target scene pointer before potentially changing current_scene_
    Scene* target_scene = scenes_[index].get(); 
    
    // Log whether it's a change or a re-selection
    if (target_scene == current_scene_) {
        if (platform_) platform_->logInfo("Theater::setScene re-selected current scene index: %zu", index);
    } else {
         if (platform_) platform_->logInfo("Theater::setScene changing to scene index: %zu", index);
         current_scene_ = target_scene; 
    }
    // ALWAYS reset and setup
    if (current_scene_) { 
        current_scene_->reset(); 
        current_scene_->setup(); 
        if (platform_) platform_->logInfo("Theater scene changed to index %zu: %s", index, current_scene_->name().c_str());
        return true;
    } else {
        // This case should ideally not happen if index check passed
        if (platform_) platform_->logError("Theater::setScene failed to set scene pointer for index %zu", index);
        return false; 
    }
}

// --- Implementation and instantiation moved back to theater.h ---
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

// // Implementation moved to header
// #include "PixelTheater/platform/web_platform.h"
// template<typename TModelDef>
// void Theater::useWebPlatform() { ... }

// // Explicit instantiation removed (not needed when definition is in header)
// #include "models/DodecaRGBv2/model.h" 
// template void Theater::useWebPlatform<PixelTheater::Models::DodecaRGBv2>();

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

} // namespace PixelTheater 


// --- Explicit Template Instantiations ---

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
// // Explicitly instantiate useWebPlatform for the specific ModelDef used in web_simulator // REMOVING
// // Include the specific model definition header required for instantiation
// #include "models/DodecaRGBv2/model.h" 
// template void PixelTheater::Theater::useWebPlatform<PixelTheater::Models::DodecaRGBv2>();
#endif 