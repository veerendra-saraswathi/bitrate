#include "bitstream.h"
#include "wavelet.h"
#include <vector>
#include <cstdint>
#include <algorithm>

static void write_val(std::vector<uint8_t>& out, int32_t val) {
    uint32_t u = (val >= 0) ? (static_cast<uint32_t>(val) << 1)
                            : (static_cast<uint32_t>(-val) << 1) - 1;
    if (u < 64) {
        out.push_back(static_cast<uint8_t>(u));
    } else {
        out.push_back(0x40 | static_cast<uint8_t>((u >> 8) & 0x3F));
        out.push_back(static_cast<uint8_t>(u & 0xFF));
    }
}

static void rice_encode(std::vector<uint8_t>& out, const std::vector<int32_t>& coeffs) {
    int run = 0;
    for (int32_t c : coeffs) {
        if (c == 0) {
            ++run;
            if (run == 255) { out.push_back(0xFF); run = 0; }
        } else {
            if (run) { out.push_back(static_cast<uint8_t>(run)); run = 0; }
            else     { out.push_back(0); }
            write_val(out, c);
        }
    }
    if (run) out.push_back(static_cast<uint8_t>(run));
}

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp) {
    FrameHeader h{};
    h.width  = static_cast<uint16_t>(width);
    h.height = static_cast<uint16_t>(height);
    h.qp     = static_cast<uint8_t>(qp);
    h.tile_size = 64;

    BitstreamWriter writer;
    writer.write_header(h);

    const int tile = 64;
    std::vector<uint8_t> payload;

    for (int ty = 0; ty < height; ty += tile) {
        for (int tx = 0; tx < width; tx += tile) {
            int tw = std::min(tile, width  - tx);
            int th = std::min(tile, height - ty);

            std::vector<int32_t> t(tw * th);
            for (int y = 0; y < th; ++y)
                for (int x = 0; x < tw; ++x)
                    t[y * tw + x] = pixels[(ty + y) * width + (tx + x)];

            wavelet_5_3_forward(t, tw, th);
            quantise(t, qp);
            rice_encode(payload, t);
        }
    }

    writer.write_bytes(payload.data(), payload.size());
    return writer.data();
}
