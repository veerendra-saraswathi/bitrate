#include "bitstream.h"
#include "wavelet.h"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
    const int W = 64, H = 64;
    std::vector<int32_t> tile(W * H);
    for (int i = 0; i < W * H; ++i) tile[i] = i % 256;

    auto original = tile;
    wavelet_5_3_forward(tile, W, H);
    wavelet_5_3_inverse(tile, W, H);

    for (size_t i = 0; i < tile.size(); ++i)
        assert(tile[i] == original[i]);

    std::cout << "5/3 wavelet round-trip OK\n";
    return 0;
}