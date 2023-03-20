cmake_minimum_required(VERSION 3.22)

project(constrained_type
    VERSION 0.0.0
    LANGUAGES CXX
)

option(BUILD_BENCHMARK "Build benchmark" TRUE)

add_library(${PROJECT_NAME} INTERFACE)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED TRUE
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (${BUILD_BENCHMARK})
    add_subdirectory("benchmark")
endif()