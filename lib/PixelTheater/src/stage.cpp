#include "PixelTheater/core/log.h"
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/stage.h"

namespace PixelTheater {

Stage::Stage() noexcept {
    // _leds = new CRGB[Model::NUM_LEDS];
}

Stage::~Stage() {
    // delete[] _leds;
}

} // namespace PixelTheater 