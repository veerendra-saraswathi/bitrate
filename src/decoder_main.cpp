#include "bitstream.h"
#include "wavelet.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <algorithm>

std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);
std::vector<int16_t> decode_frame_16(const std::vector<uint8_t>& bitstream);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: decoder <encoded.bin> [original.raw]\n";
        return 1;
    }
    std::ifstream f(argv[1], std::ios::binary);
    std::vector<uint8_t> bs((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());

    auto rec = decode_frame(bs);
    std::cout << "Decoded " << rec.size() << " bytes\n";

    // after auto rec = decode_frame(bs);

    auto rec16 = decode_frame_16(bs);   // declare the new function
    std::ofstream rec16_out("reconstructed16.raw", std::ios::binary);
    rec16_out.write(reinterpret_cast<const char*>(rec16.data()), rec16.size() * 2);
    std::cout << "Wrote reconstructed16.raw (" << rec16.size() * 2 << " bytes)\n";
    std::ofstream rec_out("reconstructed.raw", std::ios::binary);
    rec_out.write(reinterpret_cast<const char*>(rec.data()), rec.size());
    std::cout << "Wrote reconstructed.raw\n";

    if (argc >= 3) {
        std::ifstream orig(argv[2], std::ios::binary);
        std::vector<uint8_t> ref(rec.size());
        orig.read(reinterpret_cast<char*>(ref.data()), ref.size());
        int max_err = 0;
        double mse = 0;
        for (size_t i = 0; i < rec.size(); ++i) {
            int d = std::abs(int(rec[i]) - int(ref[i]));
            max_err = std::max(max_err, d);
            mse += d * d;
        }
        mse /= rec.size();
        std::cout << "max error = " << max_err
                  << "  MSE = " << mse << "\n";
    }
    return 0;
}