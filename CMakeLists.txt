cmake_minimum_required(VERSION 3.15)
project(AEG)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

set(
    VISUALIZER_SOURCES
    src/target/visualizer.cpp
)

add_executable(
    visualizer
    ${VISUALIZER_SOURCES}
)

include(FetchContent)

enable_testing()
add_subdirectory(test/unittests)
