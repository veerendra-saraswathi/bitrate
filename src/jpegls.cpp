#include "bitrate.h"
#include "jpegls.h"
#include <charls/charls.hpp>
#include <stdexcept>

std::vector<uint8_t> encode_jpegls(const std::vector<uint8_t>& pixels,
                                   int width, int height,
                                   int near_lossless)
{
    if (width <= 0 || height <= 0 ||
        pixels.size() != static_cast<size_t>(width) * height)
        throw std::invalid_argument("encode_jpegls: bad dimensions or buffer");

    charls::jpegls_encoder encoder;
    encoder.frame_info({static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height),
                        8, 1})
           .near_lossless(near_lossless)
           .interleave_mode(charls::interleave_mode::none);

    std::vector<uint8_t> dest(encoder.estimated_destination_size());
    encoder.destination(dest);

    const size_t written = encoder.encode(pixels);
    dest.resize(written);
    return dest;
}

std::vector<uint8_t> decode_jpegls(const std::vector<uint8_t>& bitstream,
                                   int& width_out, int& height_out)
{
    std::vector<uint8_t> pixels;
    const auto [info, interleave] = charls::jpegls_decoder::decode(bitstream, pixels);

    width_out  = static_cast<int>(info.width);
    height_out = static_cast<int>(info.height);

    if (info.bits_per_sample != 8 || info.component_count != 1)
        throw std::runtime_error("decode_jpegls: only 8-bit mono supported");

    return pixels;
}

std::vector<uint8_t> encode_frame_u16(const std::vector<uint16_t>& pixels,
                                      int width, int height,
                                      Architecture arch,
                                      int near)
{
    if (width <= 0 || height <= 0 ||
        pixels.size() != static_cast<size_t>(width) * height)
        throw std::invalid_argument("encode_frame_u16: bad dimensions");
    if (arch != Architecture::D)
        throw std::invalid_argument("encode_frame_u16: only Architecture D supported for 16-bit in this increment");

    charls::jpegls_encoder encoder;
    encoder.frame_info({static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height),
                        16, 1})
           .near_lossless(near)
           .interleave_mode(charls::interleave_mode::none);
    std::vector<uint8_t> dest(encoder.estimated_destination_size());
    encoder.destination(dest);
    const size_t written = encoder.encode(pixels);
    dest.resize(written);
    return dest;
}

std::vector<uint16_t> decode_frame_u16(const std::vector<uint8_t>& bitstream,
                                       int& width_out, int& height_out)
{
    std::vector<uint16_t> pixels;
    const auto [info, interleave] = charls::jpegls_decoder::decode(bitstream, pixels);
    width_out  = static_cast<int>(info.width);
    height_out = static_cast<int>(info.height);
    if (info.bits_per_sample != 16 || info.component_count != 1)
        throw std::runtime_error("decode_frame_u16: expected 16-bit mono");
    return pixels;
}
