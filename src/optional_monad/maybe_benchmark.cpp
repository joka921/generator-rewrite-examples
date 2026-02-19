// Benchmarks for optional monad coroutine vs manual implementation
#include <benchmark/benchmark.h>

#include "maybe_example.h"

// ============================================================================
// Chained Calculation Benchmarks - Success Path
// ============================================================================

static void BM_ChainedCalculation_Coro_Success(benchmark::State& state) {
  int reps = state.range(0);
  for (auto _ : state) {
    auto result = chained_calculation(10, 2, 16, reps);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Coro_Success)
    ->Arg(

        1);
BENCHMARK(BM_ChainedCalculation_Coro_Success)
    ->Arg(

        1000);
BENCHMARK(BM_ChainedCalculation_Coro_Success)
    ->Arg(

        1'000'000);

static void BM_ChainedCalculation_Coro_Header_Success(benchmark::State& state) {
  for (auto _ : state) {
    auto result = chained_calculation_header(10, 2, 16);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Coro_Header_Success);

static void BM_ChainedCalculation_Manual_Success(benchmark::State& state) {
  int reps = state.range(0);
  for (auto _ : state) {
    auto result = chained_calculation_no_coro(10, 2, 16, reps);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Manual_Success)
    ->Arg(

        1);
BENCHMARK(BM_ChainedCalculation_Manual_Success)
    ->Arg(

        1000);
BENCHMARK(BM_ChainedCalculation_Manual_Success)
    ->Arg(

        1'000'000);

// ============================================================================
// Chained Calculation Benchmarks - Divide by Zero Short-Circuit
// ============================================================================

static void BM_ChainedCalculation_Coro_DivideByZero(benchmark::State& state) {
  for (auto _ : state) {
    auto result = chained_calculation(10, 0, 16);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Coro_DivideByZero);

static void BM_ChainedCalculation_Manual_DivideByZero(benchmark::State& state) {
  for (auto _ : state) {
    auto result = chained_calculation_no_coro(10, 0, 16);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Manual_DivideByZero);

// ============================================================================
// Chained Calculation Benchmarks - Negative Sqrt Short-Circuit
// ============================================================================

static void BM_ChainedCalculation_Coro_NegativeSqrt(benchmark::State& state) {
  for (auto _ : state) {
    auto result = chained_calculation(10, 2, -4);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Coro_NegativeSqrt);

static void BM_ChainedCalculation_Manual_NegativeSqrt(benchmark::State& state) {
  for (auto _ : state) {
    auto result = chained_calculation_no_coro(10, 2, -4);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_ChainedCalculation_Manual_NegativeSqrt);

// ============================================================================
// Exception Handling Benchmarks - Success Path
// ============================================================================

static void BM_WithExceptions_Coro_Success(benchmark::State& state) {
  for (auto _ : state) {
    auto result = with_exceptions(5);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_WithExceptions_Coro_Success);

static void BM_WithExceptions_Manual_Success(benchmark::State& state) {
  for (auto _ : state) {
    auto result = with_exceptions_no_coro(5);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_WithExceptions_Manual_Success);

// ============================================================================
// Exception Handling Benchmarks - Exception Path
// ============================================================================

static void BM_WithExceptions_Coro_Exception(benchmark::State& state) {
  for (auto _ : state) {
    auto result = with_exceptions(-1);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_WithExceptions_Coro_Exception);

static void BM_WithExceptions_Manual_Exception(benchmark::State& state) {
  for (auto _ : state) {
    auto result = with_exceptions_no_coro(-1);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_WithExceptions_Manual_Exception);

// ============================================================================
// Helper Function Benchmarks - safe_divide
// ============================================================================

static void BM_SafeDivide_Success(benchmark::State& state) {
  for (auto _ : state) {
    auto result = safe_divide(10, 2);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_SafeDivide_Success);

static void BM_SafeDivide_DivideByZero(benchmark::State& state) {
  for (auto _ : state) {
    auto result = safe_divide(10, 0);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_SafeDivide_DivideByZero);

// ============================================================================
// Helper Function Benchmarks - safe_sqrt
// ============================================================================

static void BM_SafeSqrt_Success(benchmark::State& state) {
  for (auto _ : state) {
    auto result = safe_sqrt(16);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_SafeSqrt_Success);

static void BM_SafeSqrt_NegativeInput(benchmark::State& state) {
  for (auto _ : state) {
    auto result = safe_sqrt(-4);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(BM_SafeSqrt_NegativeInput);

BENCHMARK_MAIN();