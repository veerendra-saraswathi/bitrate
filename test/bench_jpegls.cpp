#include "jpegls.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>

static std::vector<uint8_t> load_raw(const std::string& path, int w, int h)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::vector<uint8_t> buf(w * h);
    f.read(reinterpret_cast<char*>(buf.data()), buf.size());
    if (!f) throw std::runtime_error("short read: " + path);
    return buf;
}

int main(int argc, char** argv)
{
    const int w = 640, h = 512;
    const std::string path = (argc > 1) ? argv[1] : "data/kaist_thermal.raw";
    const int near = (argc > 2) ? std::atoi(argv[2]) : 0;

    auto pixels = load_raw(path, w, h);

    // Warm-up
    auto enc = encode_jpegls(pixels, w, h, near);

    const int runs = 20;
    std::vector<double> times;
    times.reserve(runs);
    for (int i = 0; i < runs; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        enc = encode_jpegls(pixels, w, h, near);
        auto t1 = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }

    std::sort(times.begin(), times.end());
    double median = times[runs / 2];
    double mbps = (enc.size() * 8.0 / 1e6) * 30.0;   // 30 fps

    int dw = 0, dh = 0;
    auto dec = decode_jpegls(enc, dw, dh);
    int max_err = 0;
    for (size_t i = 0; i < pixels.size(); ++i)
        max_err = std::max(max_err, std::abs(static_cast<int>(pixels[i]) - static_cast<int>(dec[i])));

    std::cout << "NEAR=" << near
              << "  size=" << enc.size()
              << "  Mbps@30fps=" << mbps
              << "  median_ms=" << median
              << "  max_err=" << max_err << "\n";
    return 0;
}
