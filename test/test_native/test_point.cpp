#include <doctest/doctest.h>
#include "PixelTheater/limits.h"
#include "PixelTheater/model/point.h"

using namespace PixelTheater;

TEST_SUITE("Point") {
    TEST_CASE("Basic Point Construction") {
        Point p(42, 0, 1.0f, 2.0f, 3.0f);  // id, face_id, x, y, z
        
        CHECK(p.id() == 42);
        CHECK(p.face_id() == 0);
        CHECK(p.x() == 1.0f);
        CHECK(p.y() == 2.0f);
        CHECK(p.z() == 3.0f);
    }

    TEST_CASE("Point Distance Calculation") {
        Point p1(0, 0, 0.0f, 0.0f, 0.0f);  // id, face_id, x, y, z
        Point p2(1, 0, 3.0f, 4.0f, 0.0f);
        
        CHECK(p1.distanceTo(p2) == doctest::Approx(5.0f));
    }

    TEST_CASE("Point Neighbor Detection") {
        Point p1(0, 0, 0.0f, 0.0f, 0.0f);
        Point p2(1, 0, 25.0f, 0.0f, 0.0f);  // Within 30.0 threshold
        Point p3(2, 0, Limits::NEIGHBOR_THRESHOLD+1.0f, 0.0f, 0.0f);  // Beyond threshold
        Point p4(3, 1, 25.0f, 0.0f, 0.0f);  // Adjacent face
        Point p5(4, 2, 25.0f, 0.0f, 0.0f);  // Non-adjacent face
        
        CHECK(p1.isNeighbor(p2));  // Should be neighbors
        CHECK_FALSE(p1.isNeighbor(p3));  // Too far
        CHECK(p1.isNeighbor(p4));  // Adjacent face
    }
} 