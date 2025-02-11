#pragma once
#include <cstdarg>  // For va_list
#include <cstdio>   // For vprintf
#include <functional>

#ifndef PLATFORM_NATIVE
#include <Arduino.h>  // For Serial
#endif

// Log - Platform-independent logging utilities
//  - Provides consistent logging interface across native and hardware platforms
//  - Handles platform-specific output (printf vs Serial)
//  - Used for debugging and user feedback

namespace PixelTheater {
namespace Log {
    #ifdef PLATFORM_NATIVE
        // Test environment - use std::function for flexibility
        using LogFunction = std::function<void(const char*)>;
        
        inline LogFunction set_log_function(LogFunction new_func) {
            static LogFunction current_func = [](const char* msg) { printf("%s", msg); };
            auto old = current_func;
            if (new_func) current_func = new_func;
            return old;
        }
    #else
        // Hardware environment - direct to Serial
        inline void warning(const char* fmt, ...) {
            char buf[256];  // Safe buffer for formatting
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);
            Serial.print(buf);  // Print the formatted string
        }
    #endif

    // Common interface
    #ifdef PLATFORM_NATIVE
        inline void warning(const char* fmt, ...) {
            static char buffer[256];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            set_log_function(nullptr)(buffer);
        }
    #endif
}
} 