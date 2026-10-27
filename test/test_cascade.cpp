#include "wavelet.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>

static int max_abs_err(const std::vector<int32_t>& a, const std::vector<int32_t>& b) {
    int m = 0;
    for (size_t i = 0; i < a.size(); ++i)
        m = std::max(m, std::abs(a[i] - b[i]));
    return m;
}

static void check(int W, int H, int levels, const char* tag) {
    std::vector<int32_t> original(W * H);
    for (int i = 0; i < W * H; ++i)
        original[i] = (i * 17 + 3) % 256;

    auto tile = original;
    wavelet_5_3_forward(tile, W, H, levels);
    wavelet_5_3_inverse(tile, W, H, levels);

    int err = max_abs_err(tile, original);
    std::cout << tag << "  " << W << "x" << H
              << "  levels=" << levels
              << "  max_err=" << err << std::endl;
    assert(err == 0);
}

int main() {
    for (int L = 1; L <= 4; ++L)
        check(64, 64, L, "even");

    for (int L = 1; L <= 3; ++L)
        check(63, 65, L, "odd");

    check(8, 8, 1, "small");
    check(8, 8, 2, "small");
    check(7, 9, 1, "small-odd");

    std::cout << "Cascading multilevel lossless OK\n";
    return 0;
}