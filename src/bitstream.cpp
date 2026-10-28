#include "bitstream.h"
#include <cstring>
#include <algorithm>

void BitstreamWriter::write_header(const FrameHeader& h) {
    buf_.clear();
    auto push = [&](const void* p, size_t n) {
        const uint8_t* b = static_cast<const uint8_t*>(p);
        buf_.insert(buf_.end(), b, b + n);
    };
    push(&h.magic, 4);
    push(&h.version, 2);
    push(&h.header_size, 2);
    push(&h.width, 2);
    push(&h.height, 2);
    buf_.push_back(h.bit_depth);
    buf_.push_back(h.channels);
    buf_.push_back(h.tile_size);
    buf_.push_back(h.qp);
    buf_.push_back(h.levels);
    buf_.push_back(h.coder_mode);
    buf_.push_back(h.flags);
    buf_.push_back(h.reserved0);
    push(&h.sequence, 4);
    push(&h.timestamp, 8);
    buf_.insert(buf_.end(), h.reserved, h.reserved + 32);
    while (buf_.size() < h.header_size) buf_.push_back(0);
}

void BitstreamWriter::write_bytes(const uint8_t* data, size_t n) {
    buf_.insert(buf_.end(), data, data + n);
}

BitstreamReader::BitstreamReader(const std::vector<uint8_t>& data) : data_(data) {}

FrameHeader BitstreamReader::read_header() {
    FrameHeader h{};
    auto pull = [&](void* p, size_t n) {
        if (pos_ + n > data_.size()) return;
        std::memcpy(p, data_.data() + pos_, n);
        pos_ += n;
    };
    pull(&h.magic, 4);
    pull(&h.version, 2);
    pull(&h.header_size, 2);
    pull(&h.width, 2);
    pull(&h.height, 2);
    if (pos_ < data_.size()) h.bit_depth  = data_[pos_++];
    if (pos_ < data_.size()) h.channels   = data_[pos_++];
    if (pos_ < data_.size()) h.tile_size  = data_[pos_++];
    if (pos_ < data_.size()) h.qp         = data_[pos_++];
    if (pos_ < data_.size()) h.levels     = data_[pos_++];
    if (pos_ < data_.size()) h.coder_mode = data_[pos_++];
    if (pos_ < data_.size()) h.flags      = data_[pos_++];
    if (pos_ < data_.size()) h.reserved0  = data_[pos_++];
    pull(&h.sequence, 4);
    pull(&h.timestamp, 8);
    if (pos_ + 32 <= data_.size()) {
        std::memcpy(h.reserved, data_.data() + pos_, 32);
        pos_ += 32;
    }
    if (h.header_size > pos_)
        pos_ = std::min<size_t>(h.header_size, data_.size());
    return h;
}

bool BitstreamReader::read_bytes(uint8_t* dst, size_t n) {
    if (pos_ + n > data_.size()) return false;
    std::memcpy(dst, data_.data() + pos_, n);
    pos_ += n;
    return true;
}
