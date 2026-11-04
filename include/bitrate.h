#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct FrameHeader {
    uint32_t magic       = 0xB17E0001;
    uint16_t version     = 0x0002;
    uint16_t header_size = 64;
    uint16_t width       = 0;
    uint16_t height      = 0;
    uint8_t  bit_depth   = 8;
    uint8_t  channels    = 1;
    uint8_t  tile_size   = 64;
    uint8_t  qp          = 1;
    uint8_t  levels      = 1;
    uint8_t  coder_mode  = 1;
    uint8_t  flags       = 0;
    uint8_t  reserved0   = 0;
    uint32_t sequence    = 0;
    uint64_t timestamp   = 0;
    uint8_t  reserved[32] = {0};
};

enum class Architecture : int { A = 0, D = 1 };

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height,
                                  int qp = 1,
                                  int levels = 1,
                                  int coder_mode = 1,
                                  Architecture arch = Architecture::A,
                                  int near = 0);

std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

int select_qp_for_target_bpp(const std::string& curve_name, double target_bpp);

// 16-bit mono entry points (extreme-case / higher bit-depth)
std::vector<uint8_t> encode_frame_u16(const std::vector<uint16_t>& pixels,
                                      int width, int height,
                                      Architecture arch = Architecture::D,
                                      int near = 0);
std::vector<uint16_t> decode_frame_u16(const std::vector<uint8_t>& bitstream,
                                       int& width_out, int& height_out);

// Architecture A — 16-bit mono
std::vector<uint8_t> encode_frame_a_u16(const std::vector<uint16_t>& pixels,
                                        int width, int height,
                                        int qp = 1,
                                        int levels = 1,
                                        int coder_mode = 1);
std::vector<uint16_t> decode_frame_a_u16(const std::vector<uint8_t>& bitstream);
