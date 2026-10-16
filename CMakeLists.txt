cmake_minimum_required(VERSION 3.16)
project(bitrate_poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bitrate_core
    src/encoder.cpp
    src/decoder.cpp
)

target_include_directories(bitrate_core PUBLIC include)

add_executable(encoder src/encoder_main.cpp)
target_link_libraries(encoder PRIVATE bitrate_core)

add_executable(decoder src/decoder_main.cpp)
target_link_libraries(decoder PRIVATE bitrate_core)

add_executable(test_roundtrip test/test_roundtrip.cpp)
target_link_libraries(test_roundtrip PRIVATE bitrate_core)