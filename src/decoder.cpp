#include "bitstream.h"
#include "wavelet.h"

std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream) {
    BitstreamReader reader(bitstream);
    FrameHeader h = reader.read_header();

    // TODO: entropy decode, dequantise, inverse transform, reassemble tiles
    std::vector<uint8_t> out(h.width * h.height);
    // dummy fill for first link
    return out;
}