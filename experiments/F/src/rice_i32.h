#pragma once

#include <cstdint>
#include <vector>

namespace f {

// int32 Rice path for the lossy wrapper (no narrowing).
std::vector<uint8_t> rice_encode_i32(const int32_t* data, size_t n, uint8_t k);
bool rice_decode_i32(const uint8_t* in, size_t len, size_t n, uint8_t k,
                     std::vector<int32_t>& out);

// Context-adaptive wrapper that picks a good k
std::vector<uint8_t> rice_encode_i32_ctx(const int32_t* data, size_t n,
                                        int bw, int bh, uint8_t& k_used);

} // namespace f
