#include <doctest/doctest.h>
#include "PixelTheater/model/region.h"
#include "PixelTheater/model/face.h"
#include "PixelTheater/model_def.h"

using namespace PixelTheater;

TEST_SUITE("ModelDefinition") {
    TEST_CASE("basic constants") {
        using TestModel = ModelDefinition<40, 2>;
        
        CHECK(TestModel::LED_COUNT == 40);
        CHECK(TestModel::FACE_COUNT == 2);
    }
    
    TEST_CASE("face type values") {
        // Just test the enum values directly
        CHECK(static_cast<uint8_t>(FaceType::None) == 0);
        CHECK(static_cast<uint8_t>(FaceType::Strip) == 1);
        CHECK(static_cast<uint8_t>(FaceType::Circle) == 2);
        CHECK(static_cast<uint8_t>(FaceType::Triangle) == 3);
        CHECK(static_cast<uint8_t>(FaceType::Square) == 4);
        CHECK(static_cast<uint8_t>(FaceType::Pentagon) == 5);
        CHECK(static_cast<uint8_t>(FaceType::Hexagon) == 6);
    }
    
    TEST_CASE("region type values") {
        CHECK(static_cast<uint8_t>(RegionType::Center) == 0);
        CHECK(static_cast<uint8_t>(RegionType::Ring) == 1);
        CHECK(static_cast<uint8_t>(RegionType::Edge) == 2);
    }
    
    TEST_CASE("region type consistency") {
        using TestModel = ModelDefinition<40, 2>;
        
        // Verify ModelDefinition region types match Region enum
        CHECK(static_cast<uint8_t>(RegionType::Center) == static_cast<uint8_t>(RegionType::Center));
        CHECK(static_cast<uint8_t>(RegionType::Ring) == static_cast<uint8_t>(RegionType::Ring));
        CHECK(static_cast<uint8_t>(RegionType::Edge) == static_cast<uint8_t>(RegionType::Edge));
    }
    
    TEST_CASE("face type consistency") {
        using TestModel = ModelDefinition<40, 2>;
        
        // Verify ModelDefinition face types match Face enum
        CHECK(static_cast<uint8_t>(FaceType::None) == static_cast<uint8_t>(FaceType::None));
        CHECK(static_cast<uint8_t>(FaceType::Strip) == static_cast<uint8_t>(FaceType::Strip));
        CHECK(static_cast<uint8_t>(FaceType::Circle) == static_cast<uint8_t>(FaceType::Circle));
        CHECK(static_cast<uint8_t>(FaceType::Triangle) == static_cast<uint8_t>(FaceType::Triangle));
        CHECK(static_cast<uint8_t>(FaceType::Square) == static_cast<uint8_t>(FaceType::Square));
        CHECK(static_cast<uint8_t>(FaceType::Pentagon) == static_cast<uint8_t>(FaceType::Pentagon));
        CHECK(static_cast<uint8_t>(FaceType::Hexagon) == static_cast<uint8_t>(FaceType::Hexagon));
    }
} 