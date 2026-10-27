#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <numeric>

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height,
                                  int qp, int levels, int coder_mode);
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

static std::vector<uint8_t> load_raw(const std::string& path, int w, int h) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::vector<uint8_t> buf(static_cast<size_t>(w) * h);
    f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (!f) throw std::runtime_error("short read");
    return buf;
}

static double percentile(std::vector<double>& v, double p) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    double idx = p * (v.size() - 1);
    size_t i = static_cast<size_t>(idx);
    double f = idx - i;
    if (i + 1 < v.size())
        return v[i] * (1.0 - f) + v[i + 1] * f;
    return v[i];
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0]
                  << " input.raw width height qp [levels=1] [mode=1] [repeats=100]\n";
        return 1;
    }
    std::string path = argv[1];
    int width  = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    int qp     = std::stoi(argv[4]);
    int levels = argc > 5 ? std::stoi(argv[5]) : 1;
    int mode   = argc > 6 ? std::stoi(argv[6]) : 1;
    int repeats = argc > 7 ? std::stoi(argv[7]) : 100;

    auto orig = load_raw(path, width, height);

    std::vector<double> enc_ms, dec_ms;
    enc_ms.reserve(repeats);
    dec_ms.reserve(repeats);

    // warm-up
    for (int i = 0; i < 5; ++i) {
        auto bs = encode_frame(orig, width, height, qp, levels, mode);
        decode_frame(bs);
    }

    for (int i = 0; i < repeats; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto bs = encode_frame(orig, width, height, qp, levels, mode);
        auto t1 = std::chrono::high_resolution_clock::now();
        decode_frame(bs);
        auto t2 = std::chrono::high_resolution_clock::now();

        enc_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
        dec_ms.push_back(std::chrono::duration<double, std::milli>(t2 - t1).count());
    }

    auto report = [](const std::string& name, std::vector<double>& v) {
        double med = percentile(v, 0.50);
        double p95 = percentile(v, 0.95);
        double p99 = percentile(v, 0.99);
        double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        std::cout << name << ": mean=" << mean
                  << " median=" << med
                  << " p95=" << p95
                  << " p99=" << p99
                  << " ms\n";
    };

    std::cout << "QP=" << qp << " levels=" << levels << " mode=" << mode
              << " repeats=" << repeats << "\n";
    report("encode", enc_ms);
    report("decode", dec_ms);

    double med_enc = percentile(enc_ms, 0.50);
    std::cout << "approx FPS (encode)=" << (1000.0 / med_enc) << "\n";
    return 0;
}
