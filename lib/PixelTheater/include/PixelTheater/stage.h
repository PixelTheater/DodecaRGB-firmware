#pragma once
#include <cstdint>
#include <memory>
#include "model/model.h"
#include "core/crgb.h"  // Need complete type for array access
#include "platform/platform.h"

namespace PixelTheater {

class Stage {
public:
    Stage() = default;
    ~Stage() = default;

    // Platform management
    void setPlatform(std::unique_ptr<Platform> platform);
    Platform* platform() const { return _platform.get(); }

    // Model management
    void setModel(std::unique_ptr<Model> model);
    Model* model() const { return _model.get(); }

    // LED array access
    CRGB* leds();
    size_t numLeds() const;

    // Platform operations
    void show();
    void setBrightness(uint8_t brightness);
    void clear();

private:
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<Model> _model;
};

} // namespace PixelTheater 