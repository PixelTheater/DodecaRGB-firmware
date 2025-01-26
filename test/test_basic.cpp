#include "doctest/doctest.h"

TEST_CASE("Basic test framework check") {
    CHECK(true);
    
    SUBCASE("Simple math") {
        CHECK(2 + 2 == 4);
    }
} 