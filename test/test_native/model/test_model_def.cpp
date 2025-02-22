#include <doctest/doctest.h>
#include "PixelTheater/model/face.h"
#include "PixelTheater/model_def.h"
#include "../../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("ModelDefinition") {
    TEST_CASE("basic constants") {
        using TestModel = ModelDefinition<40, 2>;
        
        CHECK(TestModel::LED_COUNT == 40);
        CHECK(TestModel::FACE_COUNT == 2);
    }

    TEST_CASE("fixture data validation") {
        // Verify our test fixture has valid data
        CHECK(BasicPentagonModel::LED_COUNT == 15);
        CHECK(BasicPentagonModel::FACE_COUNT == 3);
        
        // Check face type data
        CHECK(BasicPentagonModel::FACE_TYPES[0].type == FaceType::Pentagon);
        CHECK(BasicPentagonModel::FACE_TYPES[0].num_leds == 5);
        
        // Check face instance data
        CHECK(BasicPentagonModel::FACES[0].id == 0);
        CHECK(BasicPentagonModel::FACES[0].type_id == 0);
        
        // Check point data consistency
        CHECK(BasicPentagonModel::POINTS[0].face_id == 0);  // First point on face 0
        CHECK(BasicPentagonModel::POINTS[14].face_id == 2); // Last point on face 2
    }
} 