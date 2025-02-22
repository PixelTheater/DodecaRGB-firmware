#include <doctest/doctest.h>
#include "PixelTheater/model/face.h"
#include "PixelTheater/model_def.h"

using namespace PixelTheater;

TEST_SUITE("ModelDefinition") {
    TEST_CASE("basic constants") {
        using TestModel = ModelDefinition<40, 2>;
        
        CHECK(TestModel::LED_COUNT == 40);
        CHECK(TestModel::FACE_COUNT == 2);
    }
    
    
} 