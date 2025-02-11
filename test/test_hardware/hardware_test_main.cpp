#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>

int main(int argc, char** argv) {
    doctest::Context context;

    // Disable features that might cause issues on Teensy
    context.setOption("no-breaks", true);
    context.setOption("no-path-filenames", true);
    context.setOption("no-line-numbers", true);
    context.setOption("no-skip", true);
    context.setOption("minimal", true);
    context.setOption("duration", false);
    
    context.applyCommandLine(1, argv);
    int res = context.run();
    
    return res;
} 