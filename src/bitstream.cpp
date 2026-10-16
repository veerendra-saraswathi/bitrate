#include "bitstream.h"
#include <cstring>

void BitstreamWriter::write_header(const FrameHeader& h) {
    buf_.insert(buf_.end(),
                reinterpret_cast<const uint8_t*>(&h),
                reinterpret_cast<const uint8_t*>(&h) + sizeof(FrameHeader));
}

void BitstreamWriter::write_bytes(const uint8_t* data, size_t n) {
    buf_.insert(buf_.end(), data, data + n);
}

BitstreamReader::BitstreamReader(const std::vector<uint8_t>& data) : data_(data) {}

FrameHeader BitstreamReader::read_header() {
    FrameHeader h{};
    if (pos_ + sizeof(FrameHeader) > data_.size()) return h;
    std::memcpy(&h, data_.data() + pos_, sizeof(FrameHeader));
    pos_ += sizeof(FrameHeader);
    return h;
}

bool BitstreamReader::read_bytes(uint8_t* dst, size_t n) {
    if (pos_ + n > data_.size()) return false;
    std::memcpy(dst, data_.data() + pos_, n);
    pos_ += n;
    return true;
}