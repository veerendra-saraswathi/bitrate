#pragma once
// Architecture D — JPEG-LS (CharLS) wrapper
// Pure C++ when CharLS source is compiled in. No external runtime library.

#include <cstdint>
#include <vector>

// Encode 8-bit mono frame with JPEG-LS.
// near_lossless: 0 = lossless, >0 = max absolute error (NEAR parameter).
// Returns complete JPEG-LS bitstream (SOI … EOI).
std::vector<uint8_t> encode_jpegls(const std::vector<uint8_t>& pixels,
                                   int width, int height,
                                   int near_lossless = 0);

// Decode JPEG-LS bitstream back to 8-bit mono.
// Throws std::runtime_error on malformed stream.
std::vector<uint8_t> decode_jpegls(const std::vector<uint8_t>& bitstream,
                                   int& width_out, int& height_out);
