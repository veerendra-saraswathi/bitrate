cmake_minimum_required(VERSION 3.16)
project(bitrate_poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bitrate_core
    src/bitstream.cpp
    src/wavelet.cpp
    src/encoder.cpp
    src/decoder.cpp
)

target_include_directories(bitrate_core PUBLIC include)

add_executable(test_cascade test/test_cascade.cpp)
target_link_libraries(test_cascade PRIVATE bitrate_core)

add_executable(bench_levels test/bench_levels.cpp)
target_link_libraries(bench_levels PRIVATE bitrate_core)

add_executable(test_roundtrip test/test_roundtrip.cpp)
target_link_libraries(test_roundtrip PRIVATE bitrate_core)
