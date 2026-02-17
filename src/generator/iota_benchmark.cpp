#include <benchmark/benchmark.h>
#include "./iota_unified.h"

// Callback-based iota implementation for comparison
class CallbackIota {
public:
    CallbackIota(int start, int end) : start_(start), end_(end) {}

    template<typename Callback>
    void forEach(Callback&& cb) {
        for (int i = start_; i < end_; ++i) {
            cb(i);
        }
    }

private:
    int start_;
    int end_;
};

// Benchmark for stack-allocated generator (inline_iota)
static void BM_UnifiedInlineIota(benchmark::State& state) {
    const int range = state.range(0);

    for (auto _ : state) {
        auto gen = iota_unified<false>(0, range);
        int sum = 0;
        for (int val : gen) {
            benchmark::DoNotOptimize(sum += val);
        }
        benchmark::DoNotOptimize(sum);
    }
}
// Benchmark for stack-allocated generator (inline_iota)
static void BM_UnifiedHeapIota(benchmark::State& state) {
    const int range = state.range(0);

    for (auto _ : state) {
        auto gen = iota_unified<true>(0, range);
        int sum = 0;
        for (int val : gen) {
            benchmark::DoNotOptimize(sum += val);
        }
        benchmark::DoNotOptimize(sum);
    }
}

// Benchmark for callback-based implementation
static void BM_CallbackIota(benchmark::State& state) {
    const int range = state.range(0);

    for (auto _ : state) {
        CallbackIota gen(0, range);
        int sum = 0;
        gen.forEach([&sum](int val) {
            benchmark::DoNotOptimize(sum += val);
        });
        benchmark::DoNotOptimize(sum);
    }
}

// Register benchmarks with different range sizes
BENCHMARK(BM_CallbackIota)
    ->Arg(10);
BENCHMARK(BM_UnifiedInlineIota)
    ->Arg(10);
BENCHMARK(BM_UnifiedHeapIota)
    ->Arg(10);

BENCHMARK(BM_CallbackIota)
    ->Arg(10000);
BENCHMARK(BM_UnifiedInlineIota)
    ->Arg(10000);
BENCHMARK(BM_UnifiedHeapIota)
    ->Arg(10000);

BENCHMARK_MAIN();
