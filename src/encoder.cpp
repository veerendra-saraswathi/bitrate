#include "bitstream.h"
#include "wavelet.h"
#include <fstream>
#include <iostream>

// Placeholder: real tile loop + Golomb-Rice will go here
std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp) {
    FrameHeader h;
    h.width = width;
    h.height = height;
    h.qp = qp;

    BitstreamWriter writer;
    writer.write_header(h);

    // TODO: tile, transform, quantise, entropy-code
    // For first build, just write a dummy payload so the binary links
    writer.write_bytes(pixels.data(), pixels.size() / 4); // dummy

    return writer.data();
}