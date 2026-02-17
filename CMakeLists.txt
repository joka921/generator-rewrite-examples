cmake_minimum_required(VERSION 3.16)
project(generator-rewrite-examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch Google Benchmark
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
# Don't build tests for benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(SimpleGenerators src/simple_generators.cpp)
add_executable(TaskExample src/task_example.cpp)
# add_executable(InlineGenerators src/inline_simple_generators.cpp)
add_executable(UnifiedGeneratorsTest src/unified_generators_test.cpp)

include_directories(IotaBenchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Benchmark target
add_executable(IotaBenchmark src/iota_benchmark.cpp)
target_link_libraries(IotaBenchmark PRIVATE benchmark::benchmark)

# Optional monad example (simplified version)
add_executable(MaybeMonadExample src/optional_monad/maybe_example.cpp)
target_include_directories(MaybeMonadExample PRIVATE ${CMAKE_SOURCE_DIR})
