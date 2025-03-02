#include "benchmark.h"

namespace Benchmark {

// Global benchmark data storage
std::map<std::string, BenchmarkData> benchmarks;

// Current active benchmark name
std::string current_benchmark;

// Start time for current benchmark
#if defined(PLATFORM_NATIVE) || defined(PLATFORM_WEB)
std::chrono::time_point<std::chrono::high_resolution_clock> benchmark_start_time;
#else
uint32_t benchmark_start_time;
#endif

// Whether benchmarking is enabled
bool enabled = true;

} // namespace Benchmark 