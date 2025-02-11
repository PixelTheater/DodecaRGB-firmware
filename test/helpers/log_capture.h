#pragma once
#include "PixelTheater/core/log.h"
#include <string>
#include <vector>

namespace PixelTheater {
namespace Test {

class LogCapture {
public:
    LogCapture() {
        // Save old log function and install capture
        _old_log = Log::set_log_function([this](const char* msg) {
            _messages.push_back(msg);
        });
    }

    ~LogCapture() {
        // Restore original log function
        Log::set_log_function(_old_log);
    }

    bool contains_warning() const {
        for (const auto& msg : _messages) {
            if (msg.find("[WARNING]") != std::string::npos) return true;
        }
        return false;
    }

    void clear() { _messages.clear(); }

private:
    std::vector<std::string> _messages;
    Log::LogFunction _old_log;
};

}} // namespace PixelTheater::Test 