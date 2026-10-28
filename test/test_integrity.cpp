#include "bitstream.h"
#include "rate_control.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstdint>
#include <algorithm>

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp,
                                  int levels, int coder_mode);
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

static bool bit_exact(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

int main() {
    const int W = 640, H = 512;
    const char* path = "data/kaist_visible.raw";

    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "Cannot open " << path << "\n";
        return 1;
    }
    std::vector<uint8_t> original(W * H);
    f.read(reinterpret_cast<char*>(original.data()), original.size());
    if (f.gcount() != static_cast<std::streamsize>(original.size())) {
        std::cerr << "Short read\n";
        return 1;
    }

    int qps[] = {1, 8, 32};
    int levels = 1;
    int mode = 1;

    for (int qp : qps) {
        auto bs = encode_frame(original, W, H, qp, levels, mode);

        BitstreamReader reader(bs);
        FrameHeader h = reader.read_header();
        assert(h.magic == 0xB17E0001);
        assert(h.version == 0x0002);
        assert(h.header_size == 64);
        assert(h.width == W && h.height == H);
        assert(h.qp == qp);
        assert(h.levels == levels);
        assert(h.coder_mode == mode);

        auto rec = decode_frame(bs);

        if (qp == 1) {
            if (!bit_exact(original, rec)) {
                std::cerr << "FAIL: qp=1 not bit-exact\n";
                return 1;
            }
            std::cout << "qp=1  perfect reconstruction OK  (" << bs.size() << " bytes)\n";
        } else {
            std::cout << "qp=" << qp << "  decode OK  (" << bs.size() << " bytes)\n";
        }
    }

    int qp_rc = select_qp_for_target_bpp("visible", 4.0);
    auto bs_rc = encode_frame(original, W, H, qp_rc, levels, mode);
    auto rec_rc = decode_frame(bs_rc);
    BitstreamReader r2(bs_rc);
    FrameHeader h2 = r2.read_header();
    assert(h2.qp == qp_rc);
    std::cout << "rate-control target 4.0 bpp -> QP=" << qp_rc
              << "  decode OK  (" << bs_rc.size() << " bytes)\n";

    std::cout << "All integrity checks passed.\n";
    return 0;
}
