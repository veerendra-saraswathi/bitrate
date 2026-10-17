#include "bitstream.h"
#include "wavelet.h"
#include "udp_framing.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <chrono>

std::vector<uint8_t> encode_frame(const std::vector<uint16_t>& pixels,
                                  int width, int height, int qp, int bit_depth,
                                  int channels = 1);

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: encoder <input.raw> <width> <height> [qp] [bit_depth] [channels]\n";
        return 1;
    }
    const char* path = argv[1];
    int width     = std::atoi(argv[2]);
    int height    = std::atoi(argv[3]);
    int qp        = (argc >= 5) ? std::atoi(argv[4]) : 1;
    int bit_depth = (argc >= 6) ? std::atoi(argv[5]) : 8;
    int channels  = (argc >= 7) ? std::atoi(argv[6]) : 1;

    std::ifstream f(path, std::ios::binary);
    if (!f) { std::cerr << "Cannot open " << path << "\n"; return 1; }

    size_t n = static_cast<size_t>(width) * height * channels;
    std::vector<uint16_t> pixels(n);

    if (bit_depth <= 8) {
        std::vector<uint8_t> tmp(n);
        f.read(reinterpret_cast<char*>(tmp.data()), n);
        if (f.gcount() != static_cast<std::streamsize>(n)) {
            std::cerr << "Short read\n"; return 1;
        }
        for (size_t i = 0; i < n; ++i) pixels[i] = tmp[i];
    } else {
        f.read(reinterpret_cast<char*>(pixels.data()), n * 2);
        if (f.gcount() != static_cast<std::streamsize>(n * 2)) {
            std::cerr << "Short read\n"; return 1;
        }
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    auto bitstream = encode_frame(pixels, width, height, qp, bit_depth, channels);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::ofstream out("encoded.bin", std::ios::binary);
    out.write(reinterpret_cast<const char*>(bitstream.data()), bitstream.size());

    std::cout << "Encoded " << bitstream.size() << " bytes"
              << "  (input " << (bit_depth <= 8 ? n : n * 2) << " bytes, qp=" << qp
              << ", bit_depth=" << bit_depth
              << ", channels=" << channels << ")\n";
    std::cout << "Encode time: " << ms << " ms\n";

    // UDP framing loopback
    auto packets = frame_bitstream(bitstream);
    auto recovered = reassemble_packets(packets);
    if (recovered == bitstream) {
        std::cout << "UDP framing loopback OK (" << packets.size() << " packets)\n";
    } else {
        std::cout << "UDP framing loopback FAILED\n";
        return 1;
    }

    return 0;
}