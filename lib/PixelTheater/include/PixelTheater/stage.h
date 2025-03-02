#pragma once
#include <cstdint>
#include <memory>
#include "model/model.h"
#include "core/crgb.h"  // Need complete type for array access
#include "platform/platform.h"
#include "model_def.h"

namespace PixelTheater {

template<typename ModelDef> class Scene;  // Forward declare as template

template<typename ModelDef>
class Stage {
private:
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<Model<ModelDef>> _model;  // Use template parameter
    std::vector<std::unique_ptr<Scene<ModelDef>>> _scenes;
    Scene<ModelDef>* _current_scene{nullptr};

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
    explicit Stage(std::unique_ptr<Platform> platform, std::unique_ptr<Model<ModelDef>> model)
        : _platform(std::move(platform))
        , _model(std::move(model))
        , leds{_platform->getLEDs(), ModelDef::LED_COUNT}  // Initialize leds member
        , model{*_model}  // Initialize model reference
    {}

    // Platform access
    Platform* getPlatform() const {
        return _platform.get();
    }

    // Core operations
    void update() {
        if (_current_scene) {
            _current_scene->tick();
        }
        _platform->show();
    }

    // Scene management
    template<typename T, typename... Args>
    T* addScene(Args&&... args) {
        auto scene = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = scene.get();
        _scenes.push_back(std::move(scene));
        return ptr;
    }

    void setScene(Scene<ModelDef>* scene) {
        _current_scene = scene;
    }
};

} // namespace PixelTheater 