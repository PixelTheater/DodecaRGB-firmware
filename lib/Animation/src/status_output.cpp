#include "status_output.h"
#include <cstdarg>
#include <cstdio>
#include <iomanip>  // For setprecision

namespace Animation {
    size_t StatusOutput::print(const char* str) {
        if (!str) return 0;
        _buffer << str;
        return strlen(str);
    }

    size_t StatusOutput::print(char c) {
        _buffer << c;
        return 1;
    }

    size_t StatusOutput::print(int num) {
        _buffer << num;
        return std::to_string(num).length();
    }

    size_t StatusOutput::print(float num, int decimals) {
        _buffer.precision(decimals);
        _buffer << std::fixed << num;
        return _buffer.str().length();
    }

    size_t StatusOutput::printf(const char* format, ...) {
        if (!format) return 0;
        
        va_list args;
        va_start(args, format);
        
        char buffer[256];
        int result = vsnprintf(buffer, sizeof(buffer), format, args);
        
        va_end(args);
        
        if (result >= 0) {
            _buffer << buffer;
            return result;
        }
        return 0;
    }
} // namespace Animation 