// Only define implementation and signals, other configs in platformio.ini
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include <Arduino.h>

int main(int argc, char** argv) {
    // Hardware setup
    Serial.begin(115200);
    delay(1000);  // Give serial time to connect
    
    Serial.println("\n=== Starting Hardware Tests ===");

    // Configure test runner
    doctest::Context context;
    
    // Minimize output noise
    context.setOption("no-version", true);
    context.setOption("no-intro", true);
    context.setOption("no-subcase-events", true);
    context.setOption("no-path-filenames", true);
    context.setOption("no-line-numbers", true);
    context.setOption("no-skip", true);
    context.setOption("duration", false);
    
    // Run tests
    int res = context.run();
    Serial.printf("\nTests complete with result: %d\n", res);
    
    // Keep USB alive
    while(1) { delay(100); }
    return res;
} 