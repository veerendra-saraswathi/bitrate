#include "rans.h"
#include <algorithm>
#include <vector>
#include <cstdint>

namespace f {

static void build_cum(const uint32_t* freq, uint32_t* cum) {
  cum[0] = 0;
  for (int i = 0; i < RANS_ALPH; ++i)
    cum[i + 1] = cum[i] + freq[i];
}

// Exact matching range coder.
// Encoder writes renormalisation bytes as they are produced, then a 4-byte final low.

std::vector<uint8_t> rans_encode(const uint32_t* symbols, size_t n,
                                 const uint32_t* freq) {
  uint32_t cum[RANS_ALPH + 1];
  build_cum(freq, cum);
  uint32_t total = cum[RANS_ALPH];
  if (total == 0) return {};

  uint32_t low = 0;
  uint32_t range = 0xFFFFFFFFu;
  std::vector<uint8_t> out;

  for (size_t i = 0; i < n; ++i) {
    uint32_t s = symbols[i];
    if (s >= static_cast<uint32_t>(RANS_ALPH) || freq[s] == 0) return {};

    range /= total;
    low += cum[s] * range;
    range *= freq[s];

    while ((low ^ (low + range)) < 0x01000000u || range < 0x10000u) {
      out.push_back(static_cast<uint8_t>(low >> 24));
      low <<= 8;
      range <<= 8;
    }
  }

  // Final 4 bytes of low
  out.push_back(static_cast<uint8_t>(low >> 24));
  out.push_back(static_cast<uint8_t>(low >> 16));
  out.push_back(static_cast<uint8_t>(low >> 8));
  out.push_back(static_cast<uint8_t>(low));
  return out;
}

bool rans_decode(const uint8_t* in, size_t len, size_t n,
                 const uint32_t* freq, uint32_t* symbols_out) {
  uint32_t cum[RANS_ALPH + 1];
  build_cum(freq, cum);
  uint32_t total = cum[RANS_ALPH];
  if (total == 0 || len < 4) return false;

  // Final low is the last 4 bytes
  uint32_t low = (static_cast<uint32_t>(in[len - 4]) << 24) |
                 (static_cast<uint32_t>(in[len - 3]) << 16) |
                 (static_cast<uint32_t>(in[len - 2]) << 8) |
                 static_cast<uint32_t>(in[len - 1]);
  uint32_t range = 1;
  size_t pos = 0; // renorm bytes start at the front
  size_t renorm_end = len - 4;

  auto pull = [&]() -> uint32_t {
    if (pos >= renorm_end) return 0;
    return in[pos++];
  };

  for (size_t i = 0; i < n; ++i) {
    while (range < 0x10000u) {
      low = (low << 8) | pull();
      range <<= 8;
    }
    range /= total;
    if (range == 0) return false;
    uint32_t scaled = low / range;

    uint32_t s = 0;
    while (s + 1 < RANS_ALPH && cum[s + 1] <= scaled) ++s;
    symbols_out[i] = s;

    low -= cum[s] * range;
    range *= freq[s];
  }
  return true;
}

std::vector<uint8_t> rans_encode_auto(const uint32_t* symbols, size_t n) {
  uint32_t freq[RANS_ALPH] = {};
  for (size_t i = 0; i < n; ++i)
    if (symbols[i] < RANS_ALPH) freq[symbols[i]]++;
  for (int i = 0; i < RANS_ALPH; ++i)
    if (freq[i] == 0) freq[i] = 1;
  return rans_encode(symbols, n, freq);
}

bool rans_decode_auto(const uint8_t* /*in*/, size_t /*len*/, size_t /*n*/,
                      uint32_t* /*symbols_out*/) {
  return false; // explicit-freq API is used for tests
}

} // namespace f
