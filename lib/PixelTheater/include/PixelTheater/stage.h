#pragma once
#include <cstdint>
#include <memory>
#include "model/model.h"
#include "core/crgb.h"  // Need complete type for array access
#include "platform/platform.h"
#include "model_def.h"

// Forward declare as NON-template now
// template<typename ModelDef> class Scene;  // Forward declare as template
class Scene; // Use non-templated forward declaration

namespace PixelTheater {

template<typename ModelDef>
class Stage {
private:
    Platform* _platform{nullptr};
    Model<ModelDef> _model;
    
    // Store scenes with unique_ptr
    // std::vector<std::unique_ptr<Scene<ModelDef>>> _scenes;
    // Scene<ModelDef>* _current_scene{nullptr};
    // Use non-templated Scene for now to allow compilation, will be removed later
    std::vector<std::unique_ptr<Scene>> _scenes;
    Scene* _current_scene{nullptr};

public:
    // LED array access - matches Face.h pattern
    struct Leds {
        CRGB* _data;
        uint16_t _count;

        CRGB& operator[](size_t i) const {
            if (i >= _count) i = _count - 1;  // Same bounds check as Face
            return _data[i];
        }

        CRGB* begin() const { return _data; }
        CRGB* end() const { return _data + _count; }
    } leds;  // Direct member like Face

    // Model reference access
    Model<ModelDef>& model;

    // Construction with platform and model
    explicit Stage(Platform* platform)
        : _platform(platform)
        , _model(ModelDef::createModel())
        , leds{_platform->getLEDs(), ModelDef::LED_COUNT}  // Initialize leds member
        , model{_model}  // Initialize model reference
    {}

    // Platform access
    Platform* getPlatform() const {
        return _platform;
    }

    // Core operations
    void update() { _platform->show(); } // Temp implementation without tick

    // Scene management
    // template <typename T, typename... Args>
    // T* addScene(Args&&... args) {
    //     auto scene = std::make_unique<T>(std::forward<Args>(args)...);
    //     T* scene_ptr = scene.get();
    //     _scenes.push_back(std::move(scene));
    //     if (!_current_scene) {
    //         setScene(scene_ptr);
    //     }
    //     return scene_ptr;
    // }

    // Temporarily comment out methods using Scene<ModelDef>*
    // void setScene(Scene<ModelDef>* scene) {
    //     if (scene != _current_scene) {
    //         _current_scene = scene;
    //         if (_current_scene) {
    //             _current_scene->reset();
    //             _current_scene->setup();
    //         }
    //     }
    // }

    // Scene<ModelDef>* getCurrentScene() const {
    //     return _current_scene;
    // }

    // Temporarily return nullptr or handle differently
    Scene* getCurrentScene() const {
        // TODO: Adapt or remove Stage functionality
        return nullptr; // Placeholder
    }
    
    // Get a scene by index
    Scene* getScene(size_t index) const {
        if (index < _scenes.size()) {
            return _scenes[index].get();
        }
        return nullptr;
    }
    
    // Get the number of scenes
    size_t getSceneCount() const {
        return _scenes.size();
    }
};

} // namespace PixelTheater 