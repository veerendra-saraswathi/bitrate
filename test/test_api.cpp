#include "bitrate.h"
#include <iostream>
#include <vector>
#include <cassert>

int main() {
    const int W = 64, H = 64;
    std::vector<uint8_t> px(W * H);
    for (int i = 0; i < W * H; ++i) px[i] = static_cast<uint8_t>(i % 256);

    // Explicit QP
    auto bs = encode_frame(px, W, H, /*qp=*/1);
    auto rec = decode_frame(bs);
    assert(rec == px);
    std::cout << "API: qp=1 perfect reconstruction OK (" << bs.size() << " bytes)\n";

    // Target bpp
    auto bs2 = encode_frame_target_bpp(px, W, H, 4.0, "visible");
    assert(!bs2.empty());
    auto rec2 = decode_frame(bs2);
    std::cout << "API: target-bpp path OK (" << bs2.size() << " bytes)\n";

    std::cout << "Public API smoke test passed.\n";
    return 0;
}
