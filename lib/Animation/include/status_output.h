#pragma once
#include <string>
#include <sstream>

#ifdef TEST_BUILD
#include "mock_fastled.h"
#else
#include <FastLED.h>
#endif

namespace Animation {

// Provides an Arduino-like print interface for building status messages
class StatusOutput {
public:
    // Print methods (like Arduino's Print class)
    size_t print(const char* str);
    size_t print(const std::string& str) { return print(str.c_str()); }
    size_t print(char c);
    size_t print(int num);
    size_t print(float num, int decimals = 2);
    
    // Println variants
    size_t println(const char* str) { return print(str) + print('\n'); }
    size_t println(const std::string& str) { return println(str.c_str()); }
    size_t println() { return print('\n'); }
    
    // Printf style formatting
    size_t printf(const char* format, ...);
    
    // Get and clear the buffer - not const since it modifies state
    std::string get() {
        auto result = _buffer.str();
        _buffer.str("");
        _buffer.clear();
        return result;
    }
    
    // Clear without getting - not const since it modifies state
    void clear() {
        _buffer.str("");
        _buffer.clear();
    }

private:
    std::stringstream _buffer;  // No need for mutable since methods aren't const
};

} // namespace Animation 