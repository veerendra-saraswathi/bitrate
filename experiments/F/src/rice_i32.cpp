#include "rice_i32.h"
#include <algorithm>
#include <cmath>

namespace f {

static uint32_t zigzag32(int32_t v) {
  return (static_cast<uint32_t>(v) << 1) ^ static_cast<uint32_t>(v >> 31);
}
static int32_t unzigzag32(uint32_t u) {
  return static_cast<int32_t>((u >> 1) ^ -(u & 1));
}

std::vector<uint8_t> rice_encode_i32(const int32_t* data, size_t n, uint8_t k) {
  std::vector<uint8_t> out;
  uint64_t acc = 0;
  int bits = 0;
  auto put_bit = [&](int b) {
    acc = (acc << 1) | (b & 1);
    if (++bits == 8) {
      out.push_back(static_cast<uint8_t>(acc));
      acc = 0;
      bits = 0;
    }
  };
  for (size_t i = 0; i < n; ++i) {
    uint32_t u = zigzag32(data[i]);
    uint32_t q = u >> k;
    uint32_t r = u & ((1u << k) - 1);
    for (uint32_t j = 0; j < q; ++j) put_bit(1);
    put_bit(0);
    for (int b = k - 1; b >= 0; --b) put_bit((r >> b) & 1);
  }
  if (bits) {
    acc <<= (8 - bits);
    out.push_back(static_cast<uint8_t>(acc));
  }
  return out;
}

bool rice_decode_i32(const uint8_t* in, size_t len, size_t n, uint8_t k,
                     std::vector<int32_t>& out) {
  out.resize(n);
  size_t byte_idx = 0;
  int bit_idx = 0;
  auto get_bit = [&]() -> int {
    if (byte_idx >= len) return 0;
    int b = (in[byte_idx] >> (7 - bit_idx)) & 1;
    if (++bit_idx == 8) { bit_idx = 0; ++byte_idx; }
    return b;
  };
  for (size_t i = 0; i < n; ++i) {
    uint32_t q = 0;
    while (get_bit()) ++q;
    uint32_t r = 0;
    for (int b = 0; b < k; ++b) r = (r << 1) | get_bit();
    out[i] = unzigzag32((q << k) | r);
  }
  return true;
}

std::vector<uint8_t> rice_encode_i32_ctx(const int32_t* data, size_t n,
                                        int /*bw*/, int /*bh*/, uint8_t& k_used) {
  double sum = 0;
  for (size_t i = 0; i < n; ++i) sum += std::abs(data[i]);
  double mean = (n > 0) ? sum / n : 1.0;
  int k = 0;
  while ((1 << k) < mean * 0.7 && k < 12) ++k;
  uint8_t best = static_cast<uint8_t>(k);
  size_t best_sz = SIZE_MAX;
  for (int dk = -1; dk <= 1; ++dk) {
    int kk = std::max(0, std::min(12, k + dk));
    auto tmp = rice_encode_i32(data, n, static_cast<uint8_t>(kk));
    if (tmp.size() < best_sz) {
      best_sz = tmp.size();
      best = static_cast<uint8_t>(kk);
    }
  }
  k_used = best;
  return rice_encode_i32(data, n, k_used);
}

} // namespace f
