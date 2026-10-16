#include "bitstream.h"
#include "wavelet.h"
#include <vector>
#include <cstdint>
#include <algorithm>

static int32_t read_val(const uint8_t*& p, const uint8_t* end) {
    if (p >= end) return 0;
    uint8_t b = *p++;
    uint32_t u;
    if (b & 0x40) {
        if (p >= end) return 0;
        u = (static_cast<uint32_t>(b & 0x3F) << 8) | *p++;
    } else {
        u = b;
    }
    return (u & 1) ? -static_cast<int32_t>((u + 1) >> 1)
                   :  static_cast<int32_t>(u >> 1);
}

static void rice_decode(const uint8_t*& p, const uint8_t* end,
                        std::vector<int32_t>& coeffs) {
    size_t i = 0;
    while (i < coeffs.size() && p < end) {
        uint8_t run = *p++;
        if (run == 0xFF) {
            for (int k = 0; k < 255 && i < coeffs.size(); ++k) coeffs[i++] = 0;
            continue;
        }
        for (int k = 0; k < run && i < coeffs.size(); ++k) coeffs[i++] = 0;
        if (i < coeffs.size()) coeffs[i++] = read_val(p, end);
    }
    while (i < coeffs.size()) coeffs[i++] = 0;
}

std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream) {
    BitstreamReader reader(bitstream);
    FrameHeader h = reader.read_header();

    const uint8_t* p   = bitstream.data() + sizeof(FrameHeader);
    const uint8_t* end = bitstream.data() + bitstream.size();

    std::vector<uint8_t> out(h.width * h.height);
    const int tile = h.tile_size ? h.tile_size : 64;

    for (int ty = 0; ty < h.height; ty += tile) {
        for (int tx = 0; tx < h.width; tx += tile) {
            int tw = std::min(tile, h.width  - tx);
            int th = std::min(tile, h.height - ty);

            std::vector<int32_t> t(tw * th);
            rice_decode(p, end, t);

            dequantise(t, h.qp);
            wavelet_5_3_inverse(t, tw, th);

            for (int y = 0; y < th; ++y)
                for (int x = 0; x < tw; ++x)
                    out[(ty + y) * h.width + (tx + x)] =
                        static_cast<uint8_t>(std::clamp(t[y * tw + x], 0, 255));
        }
    }
    return out;
}
