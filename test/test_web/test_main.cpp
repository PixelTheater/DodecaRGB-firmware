#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <doctest/doctest.h>

// We'll add a basic test to verify the environment works
TEST_CASE("Web environment test") {
    // This simple test should always pass
    REQUIRE(1 == 1);
}

// Add a test that checks if we are in the web environment
TEST_CASE("Verify web environment") {
    // Check if the PLATFORM_WEB is defined
    #ifdef PLATFORM_WEB
    REQUIRE(true);
    #else
    FAIL("PLATFORM_WEB not defined - not running in web environment");
    #endif
}

int main(int argc, char **argv)
{
    doctest::Context context;

    // BEGIN:: PLATFORMIO REQUIRED OPTIONS
    context.setOption("success", true);     // Report successful tests
    context.setOption("no-exitcode", true); // Do not return non-zero code on failed test case
    // END:: PLATFORMIO REQUIRED OPTIONS

    // Doctest context configuration
    context.setOption("no-version", true);
    context.setOption("no-intro", true);

    context.applyCommandLine(argc, argv);
    return context.run();
} 