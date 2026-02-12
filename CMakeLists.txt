cmake_minimum_required(VERSION 3.16)
project(generator-rewrite-examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SimpleGenerators src/simple_generators.cpp)
add_executable(TaskExample src/task_example.cpp)
