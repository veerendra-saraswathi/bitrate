#include "bitstream.h"
#include "wavelet.h"
#include <cassert>
#include <iostream>
#include <vector>

// Forward declarations
std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp);
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

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

    // Full encode → decode round-trip
    std::vector<uint8_t> img(64 * 64);
    for (int i = 0; i < 64 * 64; ++i)
        img[i] = static_cast<uint8_t>((i * 17) % 256);   // non-constant pattern

    auto bs  = encode_frame(img, 64, 64, 1);
    auto rec = decode_frame(bs);

    bool ok = true;
    for (size_t i = 0; i < img.size(); ++i)
        if (img[i] != rec[i]) { ok = false; break; }

    std::cout << "Full round-trip " << (ok ? "OK" : "FAILED")
              << "  (bitstream " << bs.size() << " bytes)\n";

    return ok ? 0 : 1;
}
