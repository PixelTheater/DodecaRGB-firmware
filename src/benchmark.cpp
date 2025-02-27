#include "benchmark.h"

namespace Benchmark {

// Initialize global variables
std::map<std::string, BenchmarkData> benchmarks;
std::string current_benchmark;
uint32_t benchmark_start_time = 0;
bool enabled = true;  // Enabled by default

} // namespace Benchmark 