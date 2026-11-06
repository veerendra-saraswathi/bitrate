#pragma once

#include <cstdint>
#include <vector>
#include "wavelet53.h"

namespace f {

struct EncodeStats;
struct SubbandStat;

bool encode_subband_blocks(const int16_t* coeffs, int width, int height,
                           int levels, int block_size,
                           std::vector<uint8_t>& out,
                           EncodeStats* stats = nullptr,
                           std::vector<SubbandStat>* sb_stats = nullptr);

bool decode_subband_blocks(const uint8_t* in, size_t len,
                           int width, int height, int levels, int block_size,
                           std::vector<int16_t>& coeffs);

} // namespace f
