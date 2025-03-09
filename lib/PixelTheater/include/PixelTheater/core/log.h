#pragma once
#include <cstdarg>  // For va_list
#include <cstdio>   // For vprintf
#include <functional>
#include <string>   // For std::string

// Define PLATFORM_WEB for Emscripten/web builds if not already defined
#ifdef EMSCRIPTEN
#ifndef PLATFORM_WEB
#define PLATFORM_WEB
#endif
#endif

// Include Arduino.h only for actual hardware platforms
#if !defined(PLATFORM_NATIVE) && !defined(PLATFORM_WEB)
#include <Arduino.h>  // For Serial
#endif

// Log - Platform-independent logging utilities
//  - Provides consistent logging interface across native and hardware platforms
//  - Handles platform-specific output (printf vs Serial)
//  - Used for debugging and user feedback

namespace PixelTheater {
namespace Log {
    // For native and web platforms - use std::function for flexibility
    #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
        using LogFunction = std::function<void(const char*)>;
        
        inline LogFunction set_log_function(LogFunction new_func) {
            static LogFunction current_func = [](const char* msg) { 
                #ifdef PLATFORM_WEB
                // In web environment, use console.log via printf
                printf("%s", msg); 
                #else
                // In native environment, use standard printf
                printf("%s", msg); 
                #endif
            };
            auto old = current_func;
            if (new_func) current_func = new_func;
            return old;
        }
        
        // Keep only the original C-style variadic function
        inline void warning(const char* fmt, ...) {
            static char buffer[256];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            set_log_function(nullptr)(buffer);
        }
        
        // Remove all template overloads
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
}
} // namespace PixelTheater 