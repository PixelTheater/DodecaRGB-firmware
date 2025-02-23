#include "PixelTheater/core/log.h"
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/stage.h"

namespace PixelTheater {

void Stage::setPlatform(std::unique_ptr<Platform> platform) {
    _platform = std::move(platform);
}

void Stage::setModel(std::unique_ptr<Model> model) {
    if (!_platform) {
        // Can't set model without platform - need LED array
        return;
    }
    
    // Initialize model with platform's LED array
    if (model && model->numLeds() <= _platform->getNumLEDs()) {
        _model = std::move(model);
        _model->setLEDs(_platform->getLEDs());
    }
}

CRGB* Stage::leds() {
    return _platform ? _platform->getLEDs() : nullptr;
}

size_t Stage::numLeds() const {
    return _platform ? _platform->getNumLEDs() : 0;
}

void Stage::show() {
    if (_platform) _platform->show();
}

void Stage::setBrightness(uint8_t brightness) {
    if (_platform) _platform->setBrightness(brightness);
}

void Stage::clear() {
    if (_platform) _platform->clear();
}

} // namespace PixelTheater 