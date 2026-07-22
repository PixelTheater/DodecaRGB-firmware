#pragma once

#include "PixelTheater/SceneKit.h"

namespace Scenes {

class IdentifySidesScene : public Scene {
public:
    static constexpr float DEFAULT_SPEED = 1.0f;
    static constexpr float DEFAULT_BRIGHTNESS = 0.8f;

    IdentifySidesScene() = default;

    void setup() override;
    void tick() override;
    std::string status() const override;

private:
    void tickFullMap(float speed, float brightness);
    void tickInteractiveWalk(float brightness);
};

} // namespace Scenes
