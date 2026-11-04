#include "bitrate.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

static std::vector<uint8_t> load_raw(const std::string& path, int w, int h) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::vector<uint8_t> px(static_cast<size_t>(w) * h);
    f.read(reinterpret_cast<char*>(px.data()), px.size());
    if (f.gcount() != static_cast<std::streamsize>(px.size()))
        throw std::runtime_error("short read: " + path);
    return px;
}

static int max_abs_err(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    int m = 0;
    for (size_t i = 0; i < a.size(); ++i)
        m = std::max(m, std::abs(static_cast<int>(a[i]) - static_cast<int>(b[i])));
    return m;
}

int main() {
    const int W = 640, H = 512;
    auto orig = load_raw("data/kaist_thermal.raw", W, H);  // adjust path if needed

    // NEAR=0: bit-exact
    {
        auto bs = encode_frame(orig, W, H, /*qp=*/1, /*levels=*/1, /*mode=*/1,
                               Architecture::D, /*near=*/0);
        auto rec = decode_frame(bs);
        assert(rec.size() == orig.size());
        int err = max_abs_err(orig, rec);
        assert(err == 0);
        std::cout << "NEAR=0 OK  bytes=" << bs.size() << "  max_err=0\n";
    }

    // NEAR=4: max_err ≤ 4
    {
        auto bs = encode_frame(orig, W, H, /*qp=*/1, /*levels=*/1, /*mode=*/1,
                               Architecture::D, /*near=*/4);
        auto rec = decode_frame(bs);
        int err = max_abs_err(orig, rec);
        assert(err <= 4);
        std::cout << "NEAR=4 OK  bytes=" << bs.size() << "  max_err=" << err << "\n";
        // Uncomment once you confirm the frozen size:
        // assert(bs.size() == 8656);
    }

    std::cout << "Architecture D public API round-trip passed\n";
    return 0;
}
