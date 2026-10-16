#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct FrameHeader {
    uint32_t magic = 0xB17E0001;   // "BITE"
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t  bit_depth = 8;
    uint8_t  channels = 1;         // 1 = mono, 3 = RGB
    uint8_t  tile_size = 128;
    uint8_t  qp = 1;               // quantisation parameter
    uint32_t sequence = 0;
    uint64_t timestamp = 0;
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
