#include "../src/rans.h"
#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <algorithm>

static int fails = 0;

static void check(const char* name, bool cond) {
  if (!cond) {
    std::cout << "FAIL: " << name << "\n";
    ++fails;
  } else {
    std::cout << "PASS: " << name << "\n";
  }
}

static void test_roundtrip(const char* name, const std::vector<uint32_t>& syms,
                           const std::vector<uint32_t>& freq) {
  auto bs = f::rans_encode(syms.data(), syms.size(), freq.data());
  std::vector<uint32_t> rec(syms.size());
  bool ok = f::rans_decode(bs.data(), bs.size(), syms.size(), freq.data(), rec.data());
  bool exact = ok && rec == syms;
  check(name, exact);
  if (!exact && !bs.empty()) {
    std::cout << "  (encoded " << bs.size() << " bytes, decode_ok=" << ok << ")\n";
  }
}

int main() {
  std::cout << "=== Independent rANS tests ===\n";

  // 1. Empty
  {
    std::vector<uint32_t> freq(f::RANS_ALPH, 1);
    test_roundtrip("empty", {}, freq);
  }

  // 2. One symbol
  {
    std::vector<uint32_t> freq(f::RANS_ALPH, 1);
    test_roundtrip("one symbol", {42}, freq);
  }

  // 3. Repeated symbols
  {
    std::vector<uint32_t> freq(f::RANS_ALPH, 1);
    freq[7] = 100;
    test_roundtrip("repeated", std::vector<uint32_t>(64, 7), freq);
  }

  // 4. All supported symbols
  {
    std::vector<uint32_t> freq(f::RANS_ALPH, 1);
    std::vector<uint32_t> all(f::RANS_ALPH);
    for (int i = 0; i < f::RANS_ALPH; ++i) all[i] = i;
    test_roundtrip("all symbols", all, freq);
  }

  // 5. Skewed
  {
    std::vector<uint32_t> freq(f::RANS_ALPH, 1);
    freq[0] = 200; freq[1] = 50;
    std::vector<uint32_t> seq(100, 0);
    for (int i = 0; i < 10; ++i) seq[i] = 1;
    test_roundtrip("skewed", seq, freq);
  }

  // 6. Random
  {
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> d(0, 15);
    std::vector<uint32_t> freq(f::RANS_ALPH, 1);
    std::vector<uint32_t> seq(256);
    for (auto& s : seq) s = d(rng);
    test_roundtrip("random", seq, freq);
  }

  std::cout << "\n" << (fails ? "SOME TESTS FAILED" : "ALL INDEPENDENT TESTS PASSED") << "\n";
  return fails ? 1 : 0;
}
