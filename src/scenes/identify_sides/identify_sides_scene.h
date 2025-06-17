#pragma once

#include "PixelTheater/SceneKit.h"

namespace Scenes {

class IdentifySidesScene : public Scene {
public:
    // === Parameter Defaults ===
    static constexpr float DEFAULT_SPEED = 1.0f;
    static constexpr float DEFAULT_BRIGHTNESS = 0.8f;

private:
    // Parameters will be accessed via settings["ParamName"]

public:
    // Constructor
    IdentifySidesScene() = default;

    // Required lifecycle methods
    void setup() override;
    void tick() override;

    // Optional status method
    std::string status() const override;
};

} // namespace Scenes 