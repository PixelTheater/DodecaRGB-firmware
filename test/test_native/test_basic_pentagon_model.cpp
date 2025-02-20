#include <doctest/doctest.h>
#include "../fixtures/models/basic_pentagon_model.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("BasicPentagonModel") {
    TEST_CASE("metadata validation") {
        // Check required metadata exists and is valid
        CHECK(BasicPentagonModel::NAME != nullptr);
        CHECK(BasicPentagonModel::VERSION != nullptr);
        CHECK(BasicPentagonModel::DESCRIPTION != nullptr);
        CHECK(BasicPentagonModel::MODEL_TYPE != nullptr);
        
        // Check content is meaningful
        CHECK(strlen(BasicPentagonModel::NAME) > 0);
        CHECK(strlen(BasicPentagonModel::VERSION) > 0);
        CHECK(strlen(BasicPentagonModel::DESCRIPTION) > 0);
        CHECK(strlen(BasicPentagonModel::MODEL_TYPE) > 0);
        
        // Verify model type matches face configuration
        CHECK(strcmp(BasicPentagonModel::MODEL_TYPE, "Pentagon") == 0);
    }
    
    TEST_CASE("face type configuration") {
        // Use the correct type reference
        CHECK(sizeof(BasicPentagonModel::FACE_TYPES) / 
              sizeof(BasicPentagonModel::FaceTypeData) == 1);
        
        const auto& face_type = BasicPentagonModel::FACE_TYPES[0];
        
        // Verify face type properties
        CHECK(face_type.id == 0);
        CHECK(face_type.type == FaceType::Pentagon);
        CHECK(face_type.num_leds == 20);
        CHECK(face_type.num_centers == 1);
        CHECK(face_type.num_rings == 3);
        CHECK(face_type.num_edges == 5);
    }
    
    TEST_CASE("face instance validation") {
        // Use the correct type reference
        CHECK(sizeof(BasicPentagonModel::FACES) / 
              sizeof(BasicPentagonModel::FaceData) == 2);
        
        const auto& face = BasicPentagonModel::FACES[0];
        
        // Verify face instance properties
        CHECK(face.id == 0);
        CHECK(face.type_id == 0);  // References the single face type
        CHECK(face.rotation == 0);  // No rotation
        
        // Face should be at origin
        CHECK(face.x == 0.0f);
        CHECK(face.y == 0.0f);
        CHECK(face.z == 1.0f);
    }
    
    TEST_CASE("center region validation") {
        // Use the correct type reference
        const BasicPentagonModel::RegionData* center_region = nullptr;
        for (const auto& region : BasicPentagonModel::REGIONS) {
            if (region.type == RegionType::Center) {
                center_region = &region;
                break;
            }
        }
        
        REQUIRE(center_region != nullptr);
        CHECK(center_region->id == 0);
        CHECK(center_region->face_id == 0);
        CHECK(center_region->led_ids[0] == 0);  // Center LED should be first
    }

    TEST_CASE("point geometry validation") {
        // According to Model.md:
        // - Every LED on a face must be a member of at least one region and define a point
        // - Points belong to faces (have face_id)
        // - Points must be within the model's coordinate system

        // Test point-to-face assignment
        for (const auto& point : BasicPentagonModel::POINTS) {
            // Every point must belong to a valid face
            CHECK(point.face_id < BasicPentagonModel::FACE_COUNT);
            
            // Points must have valid IDs within the model's LED count
            CHECK(point.id < BasicPentagonModel::LED_COUNT);
        }

        // We only need enough points to test the center region
        size_t point_count = sizeof(BasicPentagonModel::POINTS) / sizeof(BasicPentagonModel::PointData);
        CHECK(point_count >= 1);  // At least center point
    }
} 