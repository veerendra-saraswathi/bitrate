#include "bitstream.h"
#include "wavelet.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <chrono>

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp);

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: encoder <input.raw> <width> <height> [qp]\n";
        return 1;
    }
    const char* path = argv[1];
    int width  = std::atoi(argv[2]);
    int height = std::atoi(argv[3]);
    int qp     = (argc >= 5) ? std::atoi(argv[4]) : 1;

    std::ifstream f(path, std::ios::binary);
    if (!f) { std::cerr << "Cannot open " << path << "\n"; return 1; }

    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height);
    f.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
    if (f.gcount() != static_cast<std::streamsize>(pixels.size())) {
        std::cerr << "Short read\n"; return 1;
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    auto bitstream = encode_frame(pixels, width, height, qp);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::ofstream out("encoded.bin", std::ios::binary);
    out.write(reinterpret_cast<const char*>(bitstream.data()), bitstream.size());

    std::cout << "Encoded " << bitstream.size() << " bytes"
              << "  (input " << pixels.size() << " bytes, qp=" << qp << ")\n";
    std::cout << "Encode time: " << ms << " ms\n";
    return 0;
}

