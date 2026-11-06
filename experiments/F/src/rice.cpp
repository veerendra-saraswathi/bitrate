#include "rice.h"
#include <algorithm>
#include <cmath>

namespace f {

static uint32_t zigzag(int16_t v) { return (static_cast<uint32_t>(v) << 1) ^ (v >> 15); }
static int16_t unzigzag(uint32_t u) { return static_cast<int16_t>((u >> 1) ^ -(u & 1)); }

static uint8_t estimate_k_from_context(const int16_t* data, int bw, int bh, int x, int y) {
  int sum = 0, cnt = 0;
  auto add = [&](int xx, int yy) {
    if (xx >= 0 && yy >= 0 && xx < bw && yy < bh) {
      sum += std::abs(data[yy * bw + xx]);
      ++cnt;
    }
  };
  add(x - 1, y); add(x, y - 1); add(x - 1, y - 1);
  if (cnt == 0) return 2;
  double mean = static_cast<double>(sum) / cnt;
  int k = 0;
  while ((1 << k) < mean && k < 8) ++k;
  return static_cast<uint8_t>(k);
}

std::vector<uint8_t> rice_encode(const int16_t* data, size_t n, uint8_t k) {
  std::vector<uint8_t> out;
  uint32_t acc = 0; int bits = 0;
  auto put_bit = [&](int b) {
    acc = (acc << 1) | (b & 1);
    if (++bits == 8) { out.push_back(static_cast<uint8_t>(acc)); acc = 0; bits = 0; }
  };
  for (size_t i = 0; i < n; ++i) {
    uint32_t u = zigzag(data[i]);
    uint32_t q = u >> k;
    uint32_t r = u & ((1u << k) - 1);
    for (uint32_t j = 0; j < q; ++j) put_bit(1);
    put_bit(0);
    for (int b = k - 1; b >= 0; --b) put_bit((r >> b) & 1);
  }
  if (bits) { acc <<= (8 - bits); out.push_back(static_cast<uint8_t>(acc)); }
  return out;
}

std::vector<uint8_t> rice_encode_ctx(const int16_t* data, size_t n,
                                    int bw, int bh, uint8_t& k_used) {
  // Use a single k estimated from the whole block context for simplicity
  // and speed (avoids per-sample k changes that complicate the bitstream).
  double sum = 0;
  for (size_t i = 0; i < n; ++i) sum += std::abs(data[i]);
  double mean = (n > 0) ? sum / n : 1.0;
  int k = 0;
  while ((1 << k) < mean * 0.7 && k < 8) ++k;
  k_used = static_cast<uint8_t>(k);

  // Refine by trying k-1, k, k+1 and keep the smallest
  uint8_t best = k_used;
  size_t best_sz = SIZE_MAX;
  for (int dk = -1; dk <= 1; ++dk) {
    int kk = std::max(0, std::min(10, k + dk));
    auto tmp = rice_encode(data, n, static_cast<uint8_t>(kk));
    if (tmp.size() < best_sz) {
      best_sz = tmp.size();
      best = static_cast<uint8_t>(kk);
    }
  }
  k_used = best;
  return rice_encode(data, n, k_used);
}

bool rice_decode(const uint8_t* in, size_t len, size_t n, uint8_t k,
                 std::vector<int16_t>& out) {
  out.resize(n);
  size_t byte_idx = 0; int bit_idx = 0;
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
    out[i] = unzigzag((q << k) | r);
  }
  return true;
}

} // namespace f
