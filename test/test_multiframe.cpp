#include "bitrate.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <random>

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp,
                                  int levels, int coder_mode);
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

static bool bit_exact(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

static std::vector<uint8_t> make_frame(int w, int h, int seed) {
    std::vector<uint8_t> px(w * h);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& p : px) p = static_cast<uint8_t>(dist(rng));
    return px;
}

int main() {
    const int levels = 1;
    const int mode = 1;

    // ------------------------------------------------------------------
    // 1. Multi-frame sequence
    // ------------------------------------------------------------------
    {
        const int W = 640, H = 512, N = 5;
        std::vector<std::vector<uint8_t>> frames;
        std::vector<std::vector<uint8_t>> bitstreams;
        for (int i = 0; i < N; ++i) {
            frames.push_back(make_frame(W, H, 1000 + i));
            auto bs = encode_frame(frames.back(), W, H, 8, levels, mode);
            bitstreams.push_back(std::move(bs));
        }
        for (int i = 0; i < N; ++i) {
            auto rec = decode_frame(bitstreams[i]);
            assert(rec.size() == frames[i].size());
        }
        std::cout << "multi-frame sequence (" << N << " frames) OK\n";
    }

    // ------------------------------------------------------------------
    // 2. Different resolutions + perfect reconstruction at qp=1
    // ------------------------------------------------------------------
    struct Res { int w, h; const char* name; };
    Res resolutions[] = {
        {64, 64, "64x64"},
        {640, 512, "640x512"},
        {1920, 1200, "1920x1200"}
    };
    for (auto r : resolutions) {
        auto px = make_frame(r.w, r.h, 42);
        auto bs = encode_frame(px, r.w, r.h, 1, levels, mode);
        auto rec = decode_frame(bs);
        if (!bit_exact(px, rec)) {
            std::cerr << "FAIL: qp=1 not bit-exact at " << r.name << "\n";
            return 1;
        }
        std::cout << "resolution " << r.name << " qp=1 perfect OK (" << bs.size() << " bytes)\n";
    }

    // ------------------------------------------------------------------
    // 3. Truncation / malformed
    // ------------------------------------------------------------------
        // ------------------------------------------------------------------
    // 3. Truncation / malformed
    // ------------------------------------------------------------------
    {
        auto px = make_frame(64, 64, 7);
        auto bs = encode_frame(px, 64, 64, 8, levels, mode);
        if (bs.size() > 10) {
            auto trunc = std::vector<uint8_t>(bs.begin(), bs.begin() + 10);
            bool ok = true;
            try {
                auto rec = decode_frame(trunc);
                // truncated input must not produce a full-size perfect reconstruction
                if (rec.size() == px.size() && bit_exact(px, rec))
                    ok = false;
            } catch (...) {
                // throwing is also acceptable
            }
            if (!ok) {
                std::cerr << "FAIL: truncated bitstream produced a valid full frame\n";
                return 1;
            }
        }
        std::cout << "truncation check OK\n";
    }

    // ------------------------------------------------------------------
    // 4. Rate-control path on KAIST (if present)
    // ------------------------------------------------------------------
    {
        const char* path = "data/kaist_visible.raw";
        std::ifstream f(path, std::ios::binary);
        if (f) {
            const int W = 640, H = 512;
            std::vector<uint8_t> px(W * H);
            f.read(reinterpret_cast<char*>(px.data()), px.size());
            if (f.gcount() == static_cast<std::streamsize>(px.size())) {
                int qp = select_qp_for_target_bpp("visible", 4.0);
                auto bs = encode_frame(px, W, H, qp, levels, mode);
                auto rec = decode_frame(bs);
                std::cout << "rate-control on KAIST visible -> QP=" << qp
                          << " decode OK (" << bs.size() << " bytes)\n";
            }
        } else {
            std::cout << "rate-control KAIST path skipped (file missing)\n";
        }
    }

    std::cout << "All multi-frame regression checks passed.\n";
    return 0;
}
