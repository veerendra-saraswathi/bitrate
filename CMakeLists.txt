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

add_executable(decoder src/decoder_main.cpp)
target_link_libraries(decoder PRIVATE bitrate_core)

add_executable(test_regression_extra test/test_regression_extra.cpp)
target_link_libraries(test_regression_extra PRIVATE bitrate_core)





add_executable(rd_sweep test/rd_sweep.cpp)
target_link_libraries(rd_sweep PRIVATE bitrate_core)

add_executable(dump_recon test/dump_recon.cpp)
target_link_libraries(dump_recon PRIVATE bitrate_core)

add_executable(latency_probe test/latency_probe.cpp)
target_link_libraries(latency_probe PRIVATE bitrate_core)

add_executable(loss_inject test/loss_inject.cpp)
target_link_libraries(loss_inject PRIVATE bitrate_core)

# Candidate B comparator
add_library(dct_comparator src/dct.cpp)
target_include_directories(dct_comparator PUBLIC include)

add_executable(dct_rd_sweep test/dct_rd_sweep.cpp)
target_link_libraries(dct_rd_sweep PRIVATE dct_comparator)

add_executable(dct_latency_probe test/dct_latency_probe.cpp)
target_link_libraries(dct_latency_probe PRIVATE dct_comparator)

add_executable(dct_loss_test test/dct_loss_test.cpp)
target_link_libraries(dct_loss_test PRIVATE dct_comparator)

add_executable(dct_qp1_check test/dct_qp1_check.cpp)
target_link_libraries(dct_qp1_check PRIVATE dct_comparator)

add_library(pred_comparator src/pred.cpp)
target_include_directories(pred_comparator PUBLIC include)
add_executable(pred_rd_sweep test/pred_rd_sweep.cpp)
target_link_libraries(pred_rd_sweep PRIVATE pred_comparator)

# ---- Architecture D: JPEG-LS via CharLS (source compiled in) ----
include(FetchContent)
FetchContent_Declare(
  charls
  GIT_REPOSITORY https://github.com/team-charls/charls.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(charls)

target_sources(bitrate_core PRIVATE src/jpegls.cpp)
target_link_libraries(bitrate_core PUBLIC charls)

add_executable(bench_jpegls test/bench_jpegls.cpp)
target_link_libraries(bench_jpegls PRIVATE bitrate_core)
add_executable(test_dispatch test/test_dispatch.cpp)
target_link_libraries(test_dispatch PRIVATE bitrate_core)

add_executable(test_d_roundtrip test/test_d_roundtrip.cpp)
target_link_libraries(test_d_roundtrip PRIVATE bitrate_core)



add_executable(test_extreme_16bit test/test_extreme_16bit.cpp)
target_link_libraries(test_extreme_16bit PRIVATE bitrate_core)

add_executable(test_extreme_u16 test/test_extreme_u16.cpp)
target_link_libraries(test_extreme_u16 PRIVATE bitrate_core)

add_executable(test_a_u16 test/test_a_u16.cpp)
target_link_libraries(test_a_u16 PRIVATE bitrate_core)
