#include <doctest/doctest.h>
#include "PixelTheater/model/point.h"

using namespace PixelTheater;

TEST_SUITE("Point") {
    TEST_CASE("Basic Point Construction") {
        Point p(1.0f, 2.0f, 3.0f, 0, 42, 100);
        
        CHECK(p.x() == 1.0f);
        CHECK(p.y() == 2.0f);
        CHECK(p.z() == 3.0f);
        CHECK(p.face() == 0);
        CHECK(p.index() == 42);
        CHECK(p.id() == 100);
    }

    TEST_CASE("Point Distance Calculation") {
        Point p1(0.0f, 0.0f, 0.0f, 0, 0);
        Point p2(3.0f, 4.0f, 0.0f, 0, 1);
        
        CHECK(p1.distanceTo(p2) == doctest::Approx(5.0f));
    }

    TEST_CASE("Point Neighbor Detection") {
        Point p1(0.0f, 0.0f, 0.0f, 0, 0);
        Point p2(25.0f, 0.0f, 0.0f, 0, 1);  // Within 30.0 threshold
        Point p3(50.0f, 0.0f, 0.0f, 0, 2);  // Beyond threshold
        Point p4(25.0f, 0.0f, 0.0f, 1, 0);  // Adjacent face
        Point p5(25.0f, 0.0f, 0.0f, 2, 0);  // Non-adjacent face
        
        CHECK(p1.isNeighbor(p2));  // Should be neighbors
        CHECK_FALSE(p1.isNeighbor(p3));  // Too far
        CHECK(p1.isNeighbor(p4));  // Adjacent face
        CHECK_FALSE(p1.isNeighbor(p5));  // Non-adjacent face
    }
} 