#include "bitstream.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

extern std::vector<uint8_t> encode_frame(const std::vector<uint8_t>&, int, int, int, int, int);
extern std::vector<uint8_t> decode_frame(const std::vector<uint8_t>&);

int main() {
    const int W = 64, H = 64;
    std::vector<uint8_t> pixels(W * H);
    for (size_t i = 0; i < pixels.size(); ++i)
        pixels[i] = (i * 17) % 256;

    for (int mode = 0; mode <= 1; ++mode) {
        std::cout << "=== mode " << mode << " ===\n";
        for (int L = 1; L <= 4; ++L) {
            auto t0 = std::chrono::high_resolution_clock::now();
            auto bs = encode_frame(pixels, W, H, 1, L, mode);
            auto t1 = std::chrono::high_resolution_clock::now();
            auto out = decode_frame(bs);
            auto t2 = std::chrono::high_resolution_clock::now();

            int err = 0;
            for (size_t i = 0; i < out.size(); ++i)
                err = std::max(err, std::abs(int(out[i]) - int(pixels[i])));

            double enc_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            double dec_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
            std::cout << "L=" << L
                      << "  bytes=" << bs.size()
                      << "  enc=" << enc_ms << " ms"
                      << "  dec=" << dec_ms << " ms"
                      << "  max_err=" << err << "\n";
        }
    }
    return 0;
}
