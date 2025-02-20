#include <doctest/doctest.h>
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/model/point.h"
#include "PixelTheater/model/led.h"  // We'll create this

using namespace PixelTheater;

TEST_SUITE("LED Classes") {
    TEST_CASE("Led Basic Properties") {
        CRGB color;
        Point point(0, 0, 1.0f, 2.0f, 3.0f);  // id, face_id, x, y, z
        const size_t index = 42;
        
        Led led(color, point, index);
        
        // Test properties
        CHECK(led.index() == index);
        CHECK(&led.color() == &color);  // Reference to original
        CHECK(&led.point() == &point);  // Reference to original
        
        // Test color assignment
        led.color() = CRGB::Red;
        CHECK(color.r == 255);
        CHECK(color.g == 0);
        CHECK(color.b == 0);
        
        // Test point access
        CHECK(led.point().x() == 1.0f);
        CHECK(led.point().y() == 2.0f);
        CHECK(led.point().z() == 3.0f);
    }
    
    TEST_CASE("LedSpan Operations") {
    
    }
}
