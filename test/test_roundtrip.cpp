#include "bitstream.h"
#include "wavelet.h"
#include <cassert>
#include <iostream>
#include <vector>

extern std::vector<uint8_t> encode_frame(const std::vector<uint8_t>&, int, int, int, int);
extern std::vector<uint8_t> decode_frame(const std::vector<uint8_t>&);

int main() {
    const int W = 64, H = 64;

    // Wavelet-only check
    std::vector<int32_t> tile(W * H);
    for (int i = 0; i < W * H; ++i) tile[i] = i % 256;

    auto original = tile;
    wavelet_5_3_forward(tile, W, H, 1);
    wavelet_5_3_inverse(tile, W, H, 1);

    for (size_t i = 0; i < tile.size(); ++i)
        assert(tile[i] == original[i]);
    std::cout << "5/3 wavelet round-trip OK\n";

    // Full encode → decode round-trip
    std::vector<uint8_t> img(W * H);
    for (int i = 0; i < W * H; ++i)
        img[i] = static_cast<uint8_t>((i * 17) % 256);

    auto bs  = encode_frame(img, W, H, 1, 1);   // qp=1, levels=1
    auto rec = decode_frame(bs);

    bool ok = true;
    for (size_t i = 0; i < img.size(); ++i)
        if (img[i] != rec[i]) { ok = false; break; }

    std::cout << "Full round-trip " << (ok ? "OK" : "FAILED")
              << "  (bitstream " << bs.size() << " bytes)\n";

    return ok ? 0 : 1;
}
