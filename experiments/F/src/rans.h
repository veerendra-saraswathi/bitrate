#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

namespace f {

// Clean byte-oriented range coder (rANS-style) for symbols in [0, ALPH).
// Encoder and decoder are designed to be exact inverses.

static constexpr int RANS_ALPH = 256;

std::vector<uint8_t> rans_encode(const uint32_t* symbols, size_t n,
                                 const uint32_t* freq); // freq[ALPH]

bool rans_decode(const uint8_t* in, size_t len, size_t n,
                 const uint32_t* freq, uint32_t* symbols_out);

// Convenience that builds freq from the symbols themselves
std::vector<uint8_t> rans_encode_auto(const uint32_t* symbols, size_t n);
bool rans_decode_auto(const uint8_t* in, size_t len, size_t n,
                      uint32_t* symbols_out);

} // namespace f
