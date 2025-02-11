#include <doctest/doctest.h>
#include "PixelTheater/core/sentinel.h"

using namespace PixelTheater;

TEST_CASE("SentinelHandler basic functionality") {
    SUBCASE("Get sentinel values") {
        CHECK(SentinelHandler::get_sentinel<float>() == 0.0f);
        CHECK(SentinelHandler::get_sentinel<int>() == -1);
        CHECK(SentinelHandler::get_sentinel<bool>() == false);
    }

    SUBCASE("Check sentinel values") {
        CHECK(SentinelHandler::is_sentinel(0.0f));
        CHECK(SentinelHandler::is_sentinel(-1));
        CHECK(SentinelHandler::is_sentinel(false));
        
        CHECK_FALSE(SentinelHandler::is_sentinel(1.0f));
        CHECK_FALSE(SentinelHandler::is_sentinel(42));
        CHECK_FALSE(SentinelHandler::is_sentinel(true));
    }
} 