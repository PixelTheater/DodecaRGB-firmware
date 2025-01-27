#include <doctest/doctest.h>
#include "test_fixtures/space_scene_fixture.h"

TEST_SUITE("Scene Parameters") {
    TEST_CASE_FIXTURE(SpaceSceneFixture, "Space scene parameters have correct defaults") {
        CHECK(params.speed.get() == 0.5f);
        CHECK(params.brightness.get() == 0.8f);
        CHECK(params.num_particles.get() == 100);
    }
    
    TEST_CASE_FIXTURE(SpaceSceneFixture, "Parameters respect their ranges") {
        CHECK(params.speed.set(0.75f) == true);
        CHECK(params.speed.set(2.0f) == false);  // Out of range
        
        CHECK(params.num_particles.set(500) == true);
        CHECK(params.num_particles.set(-1) == false);  // Out of range
    }
} 