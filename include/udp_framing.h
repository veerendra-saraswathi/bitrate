#pragma once
#include <cstdint>
#include <vector>
#include <cstring>

struct UdpPacketHeader {
    uint32_t magic = 0x55445031;   // "UDP1"
    uint16_t seq = 0;
    uint16_t total = 0;
    uint16_t payload_len = 0;
    uint16_t flags = 0;
};

constexpr size_t UDP_PAYLOAD_MAX = 1400;   // safe for typical Ethernet MTU

std::vector<std::vector<uint8_t>> frame_bitstream(const std::vector<uint8_t>& bitstream);
std::vector<uint8_t> reassemble_packets(const std::vector<std::vector<uint8_t>>& packets);
