#pragma once
#include "animation_manager.h"
#include "hardware_display.h"

namespace Animation {

class HardwareAnimationManager : public AnimationManager {
public:
    explicit HardwareAnimationManager(const HardwareConfig& config,
                                    std::shared_ptr<TimeProvider> time = nullptr)
        : AnimationManager(time), _config(config) {}

protected:
    void setupScene(Scene* scene) override {
        auto display = std::make_unique<HardwareDisplay>(_config);
        scene->setDisplay(std::move(display));
    }

private:
    HardwareConfig _config;
};

} // namespace Animation 