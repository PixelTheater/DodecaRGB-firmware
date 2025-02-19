#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "PixelTheater/model_def.h"
#include "fixtures/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

namespace PixelTheater {
namespace Test {

TEST_SUITE("Model") {
    TEST_CASE("basic construction") {
        Model<BasicPentagonModel> model(BasicPentagonModel{});
        
        // Check metadata access
        CHECK(strcmp(model.name(), BasicPentagonModel::NAME) == 0);
        CHECK(strcmp(model.version(), BasicPentagonModel::VERSION) == 0);
        CHECK(strcmp(model.description(), BasicPentagonModel::DESCRIPTION) == 0);
    }
    
    TEST_CASE("collection access") {
        Model<BasicPentagonModel> model(BasicPentagonModel{});
        
        // Check collection sizes
        CHECK(model.faces().size() == BasicPentagonModel::FACE_COUNT);
        CHECK(model.leds().size() == BasicPentagonModel::LED_COUNT);
        
        // Check indexed access bounds handling
        CHECK_NOTHROW(model.face(0));
        CHECK_NOTHROW(model.face(BasicPentagonModel::FACE_COUNT));  // Returns first face
        CHECK_NOTHROW(model.led(0));
        CHECK_NOTHROW(model.led(BasicPentagonModel::LED_COUNT));  // Returns first LED
    }
}

} // namespace Test
} // namespace PixelTheater 