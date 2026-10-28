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
    uint8_t  coder_mode  = 1;   // 0 = zero-run, 1 = varint
    uint8_t  flags       = 0;   // bit0 = perfect-reconstruction
    uint8_t  reserved0   = 0;
    uint32_t sequence    = 0;
    uint64_t timestamp   = 0;
    uint8_t  reserved[32] = {0};
};
class BitstreamWriter {
public:
    void write_header(const FrameHeader& h);
    void write_bytes(const uint8_t* data, size_t n);
    std::vector<uint8_t> data() const { return buf_; }
private:
    std::vector<uint8_t> buf_;
};

class BitstreamReader {
public:
    explicit BitstreamReader(const std::vector<uint8_t>& data);
    FrameHeader read_header();
    bool read_bytes(uint8_t* dst, size_t n);
private:
    const std::vector<uint8_t>& data_;
    size_t pos_ = 0;
};
