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
  src/wavelet53_i32.cpp
  src/blocks.cpp
  src/rice.cpp
  src/rice_i32.cpp
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

add_executable(trace_kaist_stretch tools/trace_kaist_stretch.cpp)
target_link_libraries(trace_kaist_stretch PRIVATE f_codec)
target_include_directories(trace_kaist_stretch PRIVATE src)

add_executable(trace_kaist tools/trace_kaist.cpp)
target_link_libraries(trace_kaist PRIVATE f_codec)
target_include_directories(trace_kaist PRIVATE src)

add_executable(trace_constant tools/trace_constant.cpp)
target_link_libraries(trace_constant PRIVATE f_codec)
target_include_directories(trace_constant PRIVATE src)

add_executable(trace_wrapper_i32 tools/trace_wrapper_i32.cpp)
target_link_libraries(trace_wrapper_i32 PRIVATE f_codec)
target_include_directories(trace_wrapper_i32 PRIVATE src)

add_executable(trace_wrapper tools/trace_wrapper.cpp)
target_link_libraries(trace_wrapper PRIVATE f_codec)
target_include_directories(trace_wrapper PRIVATE src)

add_executable(trace_qstep1 tools/trace_qstep1.cpp)
target_link_libraries(trace_qstep1 PRIVATE f_codec)
target_include_directories(trace_qstep1 PRIVATE src)

add_executable(bench_lossy_corrected tools/bench_lossy_corrected.cpp)
target_link_libraries(bench_lossy_corrected PRIVATE f_codec)

add_executable(bench_lossy tools/bench_lossy.cpp)
target_link_libraries(bench_lossy PRIVATE f_codec)

add_executable(bench_14bit tools/bench_14bit.cpp)
target_link_libraries(bench_14bit PRIVATE f_codec)

add_executable(bench_kaist tools/bench_kaist.cpp)
target_link_libraries(bench_kaist PRIVATE f_codec)
