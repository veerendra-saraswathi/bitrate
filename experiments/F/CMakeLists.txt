cmake_minimum_required(VERSION 3.16)
project(architecture_F LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(f_codec STATIC
  src/wavelet53.cpp
  src/blocks.cpp
  src/rice.cpp
  src/rans.cpp
  src/encoder.cpp
  src/decoder.cpp
  src/lossy.cpp
)

target_include_directories(f_codec PUBLIC include)
target_include_directories(f_codec PRIVATE src)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(f_codec PRIVATE -O3 -DNDEBUG)
endif()

add_executable(test_rans tools/test_rans.cpp)
target_link_libraries(test_rans PRIVATE f_codec)
target_include_directories(test_rans PRIVATE src)

add_executable(bench_lossy tools/bench_lossy.cpp)
target_link_libraries(bench_lossy PRIVATE f_codec)

add_executable(bench_14bit tools/bench_14bit.cpp)
target_link_libraries(bench_14bit PRIVATE f_codec)

add_executable(bench_kaist tools/bench_kaist.cpp)
target_link_libraries(bench_kaist PRIVATE f_codec)
