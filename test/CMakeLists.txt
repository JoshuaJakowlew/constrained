cmake_minimum_required(VERSION 3.22)

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY "https://github.com/onqtam/doctest"
    GIT_TAG "master"
)

FetchContent_MakeAvailable(doctest)

file(GLOB test_SRC CONFIGURE_DEPENDS "./src/*.cpp")

add_executable(test
    ./main.cpp
    ${test_SRC}
)

set_target_properties(test PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED TRUE
)

target_include_directories(test PRIVATE
    constrained_type
    doctest
)

target_link_libraries(test
    constrained_type
    doctest
)
