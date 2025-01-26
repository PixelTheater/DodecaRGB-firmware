#pragma once
#include "base.h"
#include "../hardware/types.h"

namespace Animation {

class HardwareAnimationManager : public AnimationManager {
public:
    explicit HardwareAnimationManager(const HardwareConfig& config);
    ~HardwareAnimationManager() override = default;

protected:
    void setupScene(Scene* scene) override;

private:
    HardwareConfig _config;
};

} // namespace Animation 