#include <doctest/doctest.h>
#include "PixelTheater/model/region.h"
#include "PixelTheater/model/face.h"
#include "PixelTheater/model/model_def.h"

using namespace PixelTheater;

TEST_SUITE("ModelDefinition") {
    TEST_CASE("basic constants") {
        using TestModel = ModelDefinition<40, 2>;
        
        CHECK(TestModel::LED_COUNT == 40);
        CHECK(TestModel::FACE_COUNT == 2);
    }
    
    TEST_CASE("face types") {
        using TestModel = ModelDefinition<40, 2>;
        
        // Check face type values
        CHECK(TestModel::FaceTypes::NONE == 0);
        CHECK(TestModel::FaceTypes::STRIP == 1);
        CHECK(TestModel::FaceTypes::CIRCLE == 2);
        CHECK(TestModel::FaceTypes::TRIANGLE == 3);
        CHECK(TestModel::FaceTypes::SQUARE == 4);
        CHECK(TestModel::FaceTypes::PENTAGON == 5);
        CHECK(TestModel::FaceTypes::HEXAGON == 6);
    }
    
    TEST_CASE("region types") {
        using TestModel = ModelDefinition<40, 2>;
        
        CHECK(TestModel::RegionTypes::CENTER == 0);
        CHECK(TestModel::RegionTypes::RING == 1);
        CHECK(TestModel::RegionTypes::EDGE == 2);
    }
    
    TEST_CASE("region type consistency") {
        using TestModel = ModelDefinition<40, 2>;
        
        // Verify ModelDefinition region types match Region enum
        CHECK(static_cast<uint8_t>(RegionType::Center) == TestModel::RegionTypes::CENTER);
        CHECK(static_cast<uint8_t>(RegionType::Ring) == TestModel::RegionTypes::RING);
        CHECK(static_cast<uint8_t>(RegionType::Edge) == TestModel::RegionTypes::EDGE);
    }
    
    TEST_CASE("face type consistency") {
        using TestModel = ModelDefinition<40, 2>;
        
        // Verify ModelDefinition face types match Face enum
        CHECK(static_cast<uint8_t>(FaceType::None) == TestModel::FaceTypes::NONE);
        CHECK(static_cast<uint8_t>(FaceType::Strip) == TestModel::FaceTypes::STRIP);
        CHECK(static_cast<uint8_t>(FaceType::Circle) == TestModel::FaceTypes::CIRCLE);
        CHECK(static_cast<uint8_t>(FaceType::Triangle) == TestModel::FaceTypes::TRIANGLE);
        CHECK(static_cast<uint8_t>(FaceType::Square) == TestModel::FaceTypes::SQUARE);
        CHECK(static_cast<uint8_t>(FaceType::Pentagon) == TestModel::FaceTypes::PENTAGON);
        CHECK(static_cast<uint8_t>(FaceType::Hexagon) == TestModel::FaceTypes::HEXAGON);
    }
} 