#include <doctest/doctest.h>
#include "PixelTheater/platform/native_platform.h"
#include "PixelTheater/model/model.h"
#include "../../fixtures/models/basic_pentagon_model.h"
#include "../helpers/model_test_fixture.h"

using namespace PixelTheater;
using namespace PixelTheater::Fixtures;

TEST_SUITE("Model Points") {

    TEST_CASE("Point/Point Distance Calculation") {
        // Create a dummy model instance for testing points
        // Use the LedTestModel fixture for definition
        static PixelTheater::CRGB dummy_leds[LedTestModel::LED_COUNT];
        PixelTheater::Model<LedTestModel> model(dummy_leds);

        // Access points via model interface
        const Point& p0 = model.points[0];
        const Point& p1 = model.points[1];

        // Perform distance checks
        CHECK(p0.distanceTo(p0) == 0.0f);
        float dist = p0.distanceTo(p1);
        CHECK(dist > 0.0f);
        // Add more specific check if fixture distances are known
        // Assuming LedTestModel points are spaced 1 unit apart for simplicity
        // CHECK(dist == doctest::Approx(1.0f)); 
    }

    TEST_CASE("Point/Point Neighbor Detection") {
        static PixelTheater::CRGB dummy_leds[LedTestModel::LED_COUNT];
        PixelTheater::Model<LedTestModel> model(dummy_leds);

        // Access points via model interface
        const Point& p0 = model.points[0];
        const Point& p1 = model.points[1];
        const Point& p2 = model.points[2];
 
        // Use the isNeighbor method based on distance threshold
        CHECK(p0.isNeighbor(p1));
        CHECK(p1.isNeighbor(p0));
        CHECK(p2.isNeighbor(p1)); // Assuming p2 is also close to p1 in the fixture
    }

    TEST_CASE("Point/Neighbor Data Access") {
        // Use the LedTestModel fixture for model definition
        static PixelTheater::CRGB dummy_leds[LedTestModel::LED_COUNT];
        PixelTheater::Model<LedTestModel> model(dummy_leds); // Instantiate the actual model

        REQUIRE(model.points.size() > 0); // Ensure model has points

        // Check neighbors for the first point
        const auto& point0 = model.points[0];
        const auto& neighbors0 = point0.getNeighbors();

        // Basic sanity checks for neighbor data
        CHECK(neighbors0.size() == Limits::MAX_NEIGHBORS);

        bool found_valid_neighbor = false;
        for (size_t i = 0; i < neighbors0.size(); ++i) {
            const auto& neighbor = neighbors0[i];

            // Check if it's a potentially valid neighbor (not the sentinel)
            if (neighbor.id != 0xFFFF && neighbor.distance > 0.0f) {
                found_valid_neighbor = true;
                // Check that neighbor ID is within bounds
                CHECK(neighbor.id < model.points.size());
                CHECK(neighbor.id != point0.id()); // Point shouldn't be its own neighbor
                CHECK(neighbor.distance > 0.0f);
            } else {
                // If it's a sentinel, distance should be negative or ID invalid
                CHECK((neighbor.id == 0xFFFF || neighbor.distance <= 0.0f));
            }
        }

        // We expect at least one valid neighbor in the LedTestModel fixture
        CHECK(found_valid_neighbor);

        // Optional: Check another point if the fixture has enough points
        // Remove checks for point 1 as LedTestModel only defines neighbors for point 0
        /*
        if (model.points.size() > 1) {
            const auto& point1 = model.points[1];
            const auto& neighbors1 = point1.getNeighbors();
            bool found_valid_neighbor_1 = false;
            for (const auto& neighbor : neighbors1) {
                 if (neighbor.id != 0xFFFF && neighbor.distance > 0.0f) {
                     found_valid_neighbor_1 = true;
                     CHECK(neighbor.id < model.points.size());
                     CHECK(neighbor.id != point1.id());
                     CHECK(neighbor.distance > 0.0f);
                 }
            }
            CHECK(found_valid_neighbor_1);
        }
        */
    }
} 