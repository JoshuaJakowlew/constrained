cmake_minimum_required(VERSION 3.22)

include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG v4.3.11
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nanobench)

add_executable(benchmark
    ./main.cpp
)

set_target_properties(benchmark PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED TRUE
)

target_include_directories(benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/benchmark/include
    constrained_type
    nanobench
)

target_link_libraries(benchmark
    constrained_type
    nanobench
)
