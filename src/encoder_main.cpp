#include "bitstream.h"
#include "wavelet.h"
#include "udp_framing.h"
#include "rate_control.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <string>
#include <cstdlib>

// Existing overload used by the rest of the code (uint8 path is inside encoder.cpp)
std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp,
                                  int levels, int coder_mode);

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: encoder <input.raw> <width> <height> [qp] [bit_depth] [channels]\n"
                  << "   or: encoder <input.raw> <width> <height> --target-bpp <bpp> [--curve thermal|visible] [bit_depth] [channels]\n";
        return 1;
    }

    const char* path = argv[1];
    int width  = std::atoi(argv[2]);
    int height = std::atoi(argv[3]);

    int qp = 1;
    double target_bpp = -1.0;
    std::string curve = "thermal";
    int bit_depth = 8;
    int channels = 1;

    // Parse optional args
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--target-bpp" && i + 1 < argc) {
            target_bpp = std::atof(argv[++i]);
        } else if (arg == "--curve" && i + 1 < argc) {
            curve = argv[++i];
        } else if (arg[0] != '-') {
            // positional fallbacks: qp, bit_depth, channels
            if (qp == 1 && target_bpp < 0) qp = std::atoi(argv[i]);
            else if (bit_depth == 8) bit_depth = std::atoi(argv[i]);
            else channels = std::atoi(argv[i]);
        }
    }

    // Rate-control path
    if (target_bpp > 0) {
        qp = select_qp_for_target_bpp(curve, target_bpp);
        if (qp < 0) {
            std::cerr << "Failed to select QP for target_bpp=" << target_bpp
                      << " curve=" << curve << "\n";
            return 1;
        }
        std::cout << "Rate control: target_bpp=" << target_bpp
                  << " curve=" << curve << " -> QP=" << qp << "\n";
    }

    std::ifstream f(path, std::ios::binary);
    if (!f) { std::cerr << "Cannot open " << path << "\n"; return 1; }

    size_t n = static_cast<size_t>(width) * height * channels;
    std::vector<uint8_t> pixels(n);

    if (bit_depth <= 8) {
        f.read(reinterpret_cast<char*>(pixels.data()), n);
        if (f.gcount() != static_cast<std::streamsize>(n)) {
            std::cerr << "Short read\n"; return 1;
        }
    } else {
        // 16-bit path — promote to 8-bit for the current encoder (simple truncate)
        std::vector<uint16_t> tmp(n);
        f.read(reinterpret_cast<char*>(tmp.data()), n * 2);
        if (f.gcount() != static_cast<std::streamsize>(n * 2)) {
            std::cerr << "Short read\n"; return 1;
        }
        for (size_t i = 0; i < n; ++i) pixels[i] = static_cast<uint8_t>(tmp[i] >> 8);
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    auto bitstream = encode_frame(pixels, width, height, qp, /*levels=*/1, /*coder_mode=*/1);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::ofstream out("encoded.bin", std::ios::binary);
    out.write(reinterpret_cast<const char*>(bitstream.data()), bitstream.size());

    double bpp = (static_cast<double>(bitstream.size()) * 8.0) / n;
    std::cout << "Encoded " << bitstream.size() << " bytes"
              << "  (input " << n << " bytes, qp=" << qp
              << ", levels=1, mode=1, bit_depth=" << bit_depth
              << ", channels=" << channels << ")\n";
    std::cout << "Actual bpp: " << bpp << "\n";
    std::cout << "Encode time: " << ms << " ms\n";

    // UDP framing + timing
    auto t_concat_start = std::chrono::high_resolution_clock::now();
    auto packets = frame_bitstream(bitstream);
    auto t_frame_end = std::chrono::high_resolution_clock::now();
    double concat_ms = std::chrono::duration<double, std::milli>(t_frame_end - t_concat_start).count();
    std::cout << "Post-tile (framing): " << concat_ms << " ms\n";

    auto recovered = reassemble_packets(packets);
    if (recovered == bitstream) {
        std::cout << "UDP framing loopback OK (" << packets.size() << " packets)\n";
    } else {
        std::cout << "UDP framing loopback FAILED\n";
        return 1;
    }

    return 0;
}
