#include <doctest/doctest.h>
#include "status_output.h"

TEST_CASE("Basic StatusOutput test") {
    Animation::StatusOutput output;
    output.print("test");
    CHECK(output.get() == "test");
} 