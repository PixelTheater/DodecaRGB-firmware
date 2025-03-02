#pragma once

// Define PLATFORM_WEB for Emscripten/web builds if not already defined
#ifdef EMSCRIPTEN
#ifndef PLATFORM_WEB
#define PLATFORM_WEB
#endif
#endif

// Include appropriate headers based on platform
#if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
#include <chrono>
#include <map>
#include <string>
#include <iostream>
#else
#include <Arduino.h>
#include <map>
#include <string>
#endif

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
#if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
extern std::chrono::time_point<std::chrono::high_resolution_clock> benchmark_start_time;
#else
extern uint32_t benchmark_start_time;
#endif

// Whether benchmarking is enabled
extern bool enabled;

// Platform-specific time functions
#if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
// Get current time in microseconds
inline uint32_t micros() {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count()
    );
}
#endif

// Start a benchmark measurement
inline void start(const std::string& name) {
    if (!enabled) return;
    
    current_benchmark = name;
    #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
    benchmark_start_time = std::chrono::high_resolution_clock::now();
    #else
    benchmark_start_time = micros();
    #endif
}

// End the current benchmark measurement
inline void end() {
    if (!enabled || current_benchmark.empty()) return;
    
    uint32_t elapsed;
    #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
    auto end_time = std::chrono::high_resolution_clock::now();
    elapsed = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end_time - benchmark_start_time).count()
    );
    #else
    uint32_t end_time = micros();
    elapsed = end_time - benchmark_start_time;
    #endif
    
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
        #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
        std::cout << "No benchmark data available" << std::endl;
        #else
        Serial.println("No benchmark data available");
        #endif
        return;
    }
    
    #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
    std::cout << "\n----- BENCHMARK REPORT -----" << std::endl;
    if (fps > 0) {
        float frame_time_ms = 1000.0f / fps;
        std::cout << "FPS: " << fps << " (" << frame_time_ms << " ms/frame)" << std::endl;
    }
    
    std::cout << "Name                  | Calls |  Avg (us) |   Min   |   Max   | % Frame" << std::endl;
    std::cout << "----------------------|-------|-----------|---------|---------|--------" << std::endl;
    #else
    Serial.println("\n----- BENCHMARK REPORT -----");
    if (fps > 0) {
        float frame_time_ms = 1000.0f / fps;
        Serial.printf("FPS: %.1f (%.2f ms/frame)\n", fps, frame_time_ms);
    }
    
    Serial.println("Name                  | Calls |  Avg (us) |   Min   |   Max   | % Frame");
    Serial.println("----------------------|-------|-----------|---------|---------|--------");
    #endif
    
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
        std::string name_str = name;
        if (name_str.length() > 20) {
            name_str = name_str.substr(0, 17) + "...";
        }
        
        #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%-20s | %5d | %9.1f | %7d | %7d | %6.2f%%",
            name_str.c_str(),
            data.count,
            avg_time,
            data.min_time_us,
            data.max_time_us,
            percent
        );
        std::cout << buffer << std::endl;
        #else
        Serial.printf("%-20s | %5d | %9.1f | %7d | %7d | %6.2f%%\n",
            name_str.c_str(),
            data.count,
            avg_time,
            data.min_time_us,
            data.max_time_us,
            percent
        );
        #endif
    }
    
    #if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
    std::cout << "---------------------------" << std::endl;
    #else
    Serial.println("---------------------------");
    #endif
}

} // namespace Benchmark

// Simple macros for benchmarking
#define BENCHMARK_START(name) Benchmark::start(name)
#define BENCHMARK_END() Benchmark::end()
#define BENCHMARK_REPORT(fps) Benchmark::report(fps)
#define BENCHMARK_RESET() Benchmark::reset() 