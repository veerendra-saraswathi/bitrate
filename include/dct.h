#pragma once
#include <cstdint>
#include <vector>

// Encode one 8-bit mono frame with 8x8 DCT + uniform quant.
// qp: 1 = finest, higher = coarser (same spirit as the wavelet QP).
// Returns raw coefficient payload (not a full Bitstream v0.2 container).
std::vector<uint8_t> dct_encode_frame(const std::vector<uint8_t>& pixels,
                                      int width, int height, int qp);

// Decode back to 8-bit mono. Size must match width*height.
std::vector<uint8_t> dct_decode_frame(const std::vector<uint8_t>& payload,
                                      int width, int height, int qp);
                                      