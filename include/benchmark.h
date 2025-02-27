#pragma once

#include <Arduino.h>
#include <map>
#include <string>

namespace Benchmark {

// Structure to hold benchmark data
struct BenchmarkData {
    uint32_t total_time_us = 0;    // Total accumulated time in microseconds
    uint32_t count = 0;            // Number of times this benchmark has been run
    uint32_t min_time_us = UINT32_MAX; // Minimum time recorded
    uint32_t max_time_us = 0;      // Maximum time recorded
};

// Global benchmark data storage
extern std::map<std::string, BenchmarkData> benchmarks;

// Current active benchmark name
extern std::string current_benchmark;

// Start time for current benchmark
extern uint32_t benchmark_start_time;

// Whether benchmarking is enabled
extern bool enabled;

// Start a benchmark measurement
inline void start(const std::string& name) {
    if (!enabled) return;
    
    current_benchmark = name;
    benchmark_start_time = micros();
}

// End the current benchmark measurement
inline void end() {
    if (!enabled || current_benchmark.empty()) return;
    
    uint32_t end_time = micros();
    uint32_t elapsed = end_time - benchmark_start_time;
    
    auto& data = benchmarks[current_benchmark];
    data.total_time_us += elapsed;
    data.count++;
    
    if (elapsed < data.min_time_us) {
        data.min_time_us = elapsed;
    }
    
    if (elapsed > data.max_time_us) {
        data.max_time_us = elapsed;
    }
    
    current_benchmark.clear();
}

// Reset all benchmark data
inline void reset() {
    benchmarks.clear();
}

// Report benchmark results
inline void report(float fps = 0) {
    if (benchmarks.empty()) {
        Serial.println("No benchmark data available");
        return;
    }
    
    Serial.println("\n----- BENCHMARK REPORT -----");
    if (fps > 0) {
        float frame_time_ms = 1000.0f / fps;
        Serial.printf("FPS: %.1f (%.2f ms/frame)\n", fps, frame_time_ms);
    }
    
    Serial.println("Name                  | Calls |  Avg (us) |   Min   |   Max   | % Frame");
    Serial.println("----------------------|-------|-----------|---------|---------|--------");
    
    for (const auto& entry : benchmarks) {
        const auto& name = entry.first;
        const auto& data = entry.second;
        
        float avg_time = data.count > 0 ? (float)data.total_time_us / data.count : 0;
        float percent = 0;
        
        if (fps > 0) {
            float frame_time_us = 1000000.0f / fps;
            percent = (avg_time / frame_time_us) * 100.0f;
        }
        
        // Format the name to fit in the column (truncate if too long)
        String name_str = name.c_str();
        if (name_str.length() > 20) {
            name_str = name_str.substring(0, 17) + "...";
        }
        
        Serial.printf("%-20s | %5d | %9.1f | %7d | %7d | %6.2f%%\n",
            name_str.c_str(),
            data.count,
            avg_time,
            data.min_time_us,
            data.max_time_us,
            percent
        );
    }
    
    Serial.println("---------------------------");
}

} // namespace Benchmark

// Simple macros for benchmarking
#define BENCHMARK_START(name) Benchmark::start(name)
#define BENCHMARK_END() Benchmark::end()
#define BENCHMARK_REPORT(fps) Benchmark::report(fps)
#define BENCHMARK_RESET() Benchmark::reset() 