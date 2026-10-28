cmake_minimum_required(VERSION 3.16)
project(bitrate_poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bitrate_core
    src/bitstream.cpp
    src/wavelet.cpp
    src/encoder.cpp
    src/decoder.cpp
    src/rate_control.cpp
    src/udp_framing.cpp
)

target_include_directories(bitrate_core PUBLIC include)

add_executable(encoder src/encoder_main.cpp)
target_link_libraries(encoder PRIVATE bitrate_core)

add_executable(test_cascade test/test_cascade.cpp)
target_link_libraries(test_cascade PRIVATE bitrate_core)

add_executable(bench_levels test/bench_levels.cpp)
target_link_libraries(bench_levels PRIVATE bitrate_core)

add_executable(test_roundtrip test/test_roundtrip.cpp)
target_link_libraries(test_roundtrip PRIVATE bitrate_core)

add_executable(bench_quality test/bench_quality.cpp)
target_link_libraries(bench_quality PRIVATE bitrate_core)

add_executable(bench_jetson test/bench_jetson.cpp)
target_link_libraries(bench_jetson PRIVATE bitrate_core)

add_executable(rate_control_demo test/rate_control_demo.cpp)
target_link_libraries(rate_control_demo PRIVATE bitrate_core)

add_executable(test_integrity test/test_integrity.cpp)
target_link_libraries(test_integrity PRIVATE bitrate_core)

add_executable(test_api test/test_api.cpp)
target_link_libraries(test_api PRIVATE bitrate_core)

add_executable(test_multiframe test/test_multiframe.cpp)
target_link_libraries(test_multiframe PRIVATE bitrate_core)
