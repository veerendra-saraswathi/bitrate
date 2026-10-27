#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <algorithm>

// Library functions
std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height,
                                  int qp, int levels, int coder_mode);
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

static double psnr(const std::vector<uint8_t>& orig,
                   const std::vector<uint8_t>& rec) {
    if (orig.size() != rec.size() || orig.empty()) return 0.0;
    double mse = 0.0;
    for (size_t i = 0; i < orig.size(); ++i) {
        double d = static_cast<double>(orig[i]) - static_cast<double>(rec[i]);
        mse += d * d;
    }
    mse /= static_cast<double>(orig.size());
    if (mse < 1e-10) return 99.0;
    return 10.0 * std::log10(255.0 * 255.0 / mse);
}

// Simple SSIM (8x8 windows, mean of local SSIMs)
static double ssim(const std::vector<uint8_t>& orig,
                   const std::vector<uint8_t>& rec,
                   int width, int height) {
    if (orig.size() != rec.size() || width <= 0 || height <= 0) return 0.0;
    const int win = 8;
    const double C1 = 6.5025;   // (0.01*255)^2
    const double C2 = 58.5225;  // (0.03*255)^2
    double sum = 0.0;
    int count = 0;

    for (int y = 0; y + win <= height; y += win) {
        for (int x = 0; x + win <= width; x += win) {
            double mu_o = 0, mu_r = 0;
            for (int wy = 0; wy < win; ++wy)
                for (int wx = 0; wx < win; ++wx) {
                    int idx = (y + wy) * width + (x + wx);
                    mu_o += orig[idx];
                    mu_r += rec[idx];
                }
            mu_o /= (win * win);
            mu_r /= (win * win);

            double sig_o = 0, sig_r = 0, sig_or = 0;
            for (int wy = 0; wy < win; ++wy)
                for (int wx = 0; wx < win; ++wx) {
                    int idx = (y + wy) * width + (x + wx);
                    double do_ = orig[idx] - mu_o;
                    double dr = rec[idx] - mu_r;
                    sig_o += do_ * do_;
                    sig_r += dr * dr;
                    sig_or += do_ * dr;
                }
            sig_o /= (win * win - 1);
            sig_r /= (win * win - 1);
            sig_or /= (win * win - 1);

            double num = (2 * mu_o * mu_r + C1) * (2 * sig_or + C2);
            double den = (mu_o * mu_o + mu_r * mu_r + C1) * (sig_o + sig_r + C2);
            sum += num / den;
            ++count;
        }
    }
    return count ? sum / count : 0.0;
}

static std::vector<uint8_t> load_raw(const std::string& path, int w, int h) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::vector<uint8_t> buf(static_cast<size_t>(w) * h);
    f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (!f) throw std::runtime_error("short read from " + path);
    return buf;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " input.raw width height [levels=3] [mode=1]\n";
        return 1;
    }

    std::string path = argv[1];
    int width  = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    int levels = (argc > 4) ? std::stoi(argv[4]) : 1;
    // default levels=1 (best RD on KAIST thermal)
    int mode   = (argc > 5) ? std::stoi(argv[5]) : 1;

    auto orig = load_raw(path, width, height);
    const double pixels = static_cast<double>(width) * height;

    std::cout << "QP,bytes,bpp,enc_ms,psnr_dB,ssim\n";

    for (int qp = 1; qp <= 32; ++qp) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto bs = encode_frame(orig, width, height, qp, levels, mode);
        auto t1 = std::chrono::high_resolution_clock::now();
        double enc_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        auto rec = decode_frame(bs);
        double p = psnr(orig, rec);
        double s = ssim(orig, rec, width, height);
        double bpp = (static_cast<double>(bs.size()) * 8.0) / pixels;

        std::cout << qp << ","
                  << bs.size() << ","
                  << bpp << ","
                  << enc_ms << ","
                  << p << ","
                  << s << "\n";
    }
    return 0;
}
