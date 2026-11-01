#include "dct.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::vector<uint8_t> load_raw(const std::string& path, int w, int h) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::vector<uint8_t> buf(static_cast<size_t>(w) * h);
    f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (!f) throw std::runtime_error("short read");
    return buf;
}
double psnr_8(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0;
    double mse = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        mse += d * d;
    }
    mse /= static_cast<double>(a.size());
    if (mse < 1e-12) return 99.0;
    return 10.0 * std::log10(255.0 * 255.0 / mse);
}
int max_abs_err(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    int m = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        int d = std::abs(static_cast<int>(a[i]) - static_cast<int>(b[i]));
        if (d > m) m = d;
    }
    return m;
}
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input.raw> <width> <height> [fps=30] [max_qp=32]\n";
        return 1;
    }
    const std::string path = argv[1];
    const int width  = std::stoi(argv[2]);
    const int height = std::stoi(argv[3]);
    const double fps = (argc > 4) ? std::stod(argv[4]) : 30.0;
    const int max_qp = (argc > 5) ? std::stoi(argv[5]) : 32;

    auto orig = load_raw(path, width, height);
    const double pixels = static_cast<double>(width) * height;

    std::cout << "qp,bytes,bpp,mbps_at_" << fps << "fps,psnr_dB,max_err,encode_ms,decode_ms\n";

    for (int qp = 1; qp <= max_qp; ++qp) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto payload = dct_encode_frame(orig, width, height, qp);
        auto t1 = std::chrono::high_resolution_clock::now();

        auto t2 = std::chrono::high_resolution_clock::now();
        auto recon = dct_decode_frame(payload, width, height, qp);
        auto t3 = std::chrono::high_resolution_clock::now();

        const double encode_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        const double decode_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
        const double bpp  = (static_cast<double>(payload.size()) * 8.0) / pixels;
        const double mbps = (static_cast<double>(payload.size()) * 8.0 * fps) / 1e6;

        std::cout << qp << "," << payload.size() << "," << bpp << ","
                  << mbps << "," << psnr_8(orig, recon) << ","
                  << max_abs_err(orig, recon) << ","
                  << encode_ms << "," << decode_ms << "\n";
    }
    return 0;
}
