#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "../fixtures/models/basic_pentagon_model.h"

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
        CHECK_NOTHROW(model[0]);  // Use operator[] instead of face()
        CHECK_NOTHROW(model[BasicPentagonModel::FACE_COUNT]);  // Returns first face
        CHECK_NOTHROW(model.led(0));
        CHECK_NOTHROW(model.led(BasicPentagonModel::LED_COUNT));  // Returns first LED
    }
    
    TEST_CASE("face access") {
        Model<BasicPentagonModel> model(BasicPentagonModel{});
        
        // Test collection access
        auto faces = model.faces();
        CHECK(faces.size() == BasicPentagonModel::FACE_COUNT);
        
        // Test indexed access via operator[]
        CHECK_NOTHROW(model[0]);  // First face
        CHECK_NOTHROW(model[BasicPentagonModel::FACE_COUNT]);  // Out of bounds returns first face
        
        // Test range-based for
        size_t count = 0;
        for(const auto& face : model.faces()) {
            CHECK(face.type() == FaceType::Pentagon);
            count++;
        }
        CHECK(count == BasicPentagonModel::FACE_COUNT);
    }
    
    TEST_CASE("region initialization") {
        Model<BasicPentagonModel> model(BasicPentagonModel{});
        
        // Get first face
        const auto& face = model[0];
        
        // Check center region
        CHECK(face.center().type() == RegionType::Center);
        CHECK(face.center().leds().size() == 1);  // Single center LED
        
        // Check rings
        auto rings = face.rings();
        CHECK(rings.size() == 1);  // One ring from fixture
        CHECK(rings[0].type() == RegionType::Ring);
        CHECK(rings[0].leds().size() == 5);  // 5 LEDs in first ring
        
        // Check edges
        auto edges = face.edges();
        CHECK(edges.size() == 3);  // Three edges from fixture
        CHECK(edges[0].type() == RegionType::Edge);
        CHECK(edges[0].leds().size() == 5);  // 5 LEDs per edge
    }
}

} // namespace Test
} // namespace PixelTheater 