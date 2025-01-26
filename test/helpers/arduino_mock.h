#pragma once
#include <chrono>

namespace Animation {
    // Mock basic Arduino functions
    unsigned long millis() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }
    
    void delay(unsigned long ms) {
        // No-op for tests
    }
} // namespace Animation 