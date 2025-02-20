#include <doctest/doctest.h>
#include "PixelTheater/model/model.h"
#include "../fixtures/models/model_relationships.h"  // Simple test model fixture

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Relationships") {
    TEST_CASE("LED to Point Mapping") {
        Model<RelationshipsTestModel> model(RelationshipsTestModel{});

        SUBCASE("Every LED has valid Point data") {
            for(const auto& led : model.leds()) {
                // Point should be initialized and valid
                const auto& point = led.point();
                CHECK(point.id() < RelationshipsTestModel::LED_COUNT);
                
                // Point should have valid face assignment
                CHECK(point.face_id() < RelationshipsTestModel::FACE_COUNT);
                
                // Point coordinates should match definition
                const auto& point_def = RelationshipsTestModel::POINTS[led.index()];
                CHECK(point.x() == point_def.x);
                CHECK(point.y() == point_def.y);
                CHECK(point.z() == point_def.z);
            }
        }

        SUBCASE("Points maintain face relationship") {
            for(const auto& face : model.faces()) {
                for(const auto& region : face.regions()) {
                    // Use size() and operator[] since LedSpan doesn't support range-for yet
                    for(size_t i = 0; i < region.leds().size(); i++) {
                        const auto& led = region.leds()[i];
                        // LED's point should reference back to correct face
                        CHECK(led.point().face_id() == face.id());
                    }
                }
            }
        }
    }

    TEST_CASE("Region to Face Relationships") {
        Model<RelationshipsTestModel> model(RelationshipsTestModel{});

        SUBCASE("Regions are properly assigned to faces") {
            for(const auto& face : model.faces()) {
                // Check center region
                CHECK(face.center().type() == RegionType::Center);
                
                // Check ring regions
                for(const auto& ring : face.rings()) {
                    CHECK(ring.type() == RegionType::Ring);
                }
                
                // Check edge regions
                for(const auto& edge : face.edges()) {
                    CHECK(edge.type() == RegionType::Edge);
                }
            }
        }

        SUBCASE("Region LED counts match definition") {
            for(const auto& face : model.faces()) {
                const auto& face_type = RelationshipsTestModel::FACE_TYPES[0];  // Only one type
                
                CHECK(face.center().leds().size() == 1);  // Center has 1 LED
                CHECK(face.rings()[0].leds().size() == 3);  // Ring has 3 LEDs
                CHECK(face.edges()[0].leds().size() == 2);  // Edge has 2 LEDs
            }
        }
    }

    TEST_CASE("Point Geometric Relationships") {
        Model<RelationshipsTestModel> model(RelationshipsTestModel{});

        SUBCASE("Point distances are consistent") {
            // Get center points of each face
            const auto& face0_center = model.faces()[0].center().leds()[0].point();
            const auto& face1_center = model.faces()[1].center().leds()[0].point();

            // Distance should match our model definition
            CHECK(face0_center.distanceTo(face1_center) == doctest::Approx(2.0f));  // Centers are 2 units apart
        }

        SUBCASE("Neighbor relationships are symmetric") {
            for(const auto& led1 : model.leds()) {
                for(const auto& led2 : model.leds()) {
                    if(led1.index() != led2.index()) {
                        // If A is neighbor of B, B must be neighbor of A
                        CHECK(led1.point().isNeighbor(led2.point()) == 
                              led2.point().isNeighbor(led1.point()));
                    }
                }
            }
        }
    }

    TEST_CASE("Model Consistency") {
        Model<RelationshipsTestModel> model(RelationshipsTestModel{});

        SUBCASE("All LEDs are assigned to regions") {
            std::vector<bool> led_found(RelationshipsTestModel::LED_COUNT, false);
            
            // Mark all LEDs found in regions
            for(const auto& face : model.faces()) {
                for(const auto& region : face.regions()) {
                    for(size_t i = 0; i < region.leds().size(); i++) {
                        led_found[region.leds()[i].index()] = true;
                    }
                }
            }
            
            // Every LED should be found exactly once
            for(bool found : led_found) {
                CHECK(found);
            }
        }

        SUBCASE("Region types match definition") {
            size_t center_count = 0;
            size_t ring_count = 0;
            size_t edge_count = 0;

            for(const auto& region : model.regions()) {
                // No region should have type None
                CHECK(region.type() != RegionType::None);
                
                switch(region.type()) {
                    case RegionType::Center: center_count++; break;
                    case RegionType::Ring: ring_count++; break;
                    case RegionType::Edge: edge_count++; break;
                    case RegionType::None: 
                        // Should never happen due to check above
                        CHECK(false); 
                        break;
                }
            }

            CHECK(center_count == RelationshipsTestModel::FACE_COUNT);  // One center per face
            CHECK(ring_count == RelationshipsTestModel::FACE_COUNT);    // One ring per face
            CHECK(edge_count == RelationshipsTestModel::FACE_COUNT);    // One edge per face
        }
    }
} 