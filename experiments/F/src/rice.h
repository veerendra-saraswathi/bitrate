#pragma once

#include <cstdint>
#include <vector>

namespace f {

// Context-adaptive Rice: k chosen from local magnitude context.
// Falls back to exhaustive search only if needed.

std::vector<uint8_t> rice_encode_ctx(const int16_t* data, size_t n,
                                    int bw, int bh, uint8_t& k_used);
bool rice_decode(const uint8_t* in, size_t len, size_t n, uint8_t k,
                 std::vector<int16_t>& out);

// Legacy fixed-k encode (kept for compatibility)
std::vector<uint8_t> rice_encode(const int16_t* data, size_t n, uint8_t k);

} // namespace f
