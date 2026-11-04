#pragma once
#include "bitrate.h"
#include <cstdint>
#include <vector>

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
