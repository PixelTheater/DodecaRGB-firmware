#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char** argv) {
    doctest::Context context;
    
    // BEGIN:: PLATFORMIO REQUIRED OPTIONS
    context.setOption("success", true);     // Reports successful tests
    context.setOption("no-exitcode", true); // Do not return non-zero code on failed test case
    // END:: PLATFORMIO REQUIRED OPTIONS
    
    context.applyCommandLine(argc, argv);
    return context.run();
} 