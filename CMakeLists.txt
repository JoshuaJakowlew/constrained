cmake_minimum_required(VERSION 3.22)

project(constrained_type
    VERSION 0.0.0
    LANGUAGES CXX
)

# add_library(${PROJECT_NAME} INTERFACE)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./include
)