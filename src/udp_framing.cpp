#include "udp_framing.h"
#include <algorithm>

std::vector<std::vector<uint8_t>> frame_bitstream(const std::vector<uint8_t>& bitstream) {
    std::vector<std::vector<uint8_t>> packets;
    size_t offset = 0;
    uint16_t seq = 0;
    uint16_t total = static_cast<uint16_t>((bitstream.size() + UDP_PAYLOAD_MAX - 1) / UDP_PAYLOAD_MAX);

    while (offset < bitstream.size()) {
        size_t chunk = std::min(UDP_PAYLOAD_MAX, bitstream.size() - offset);
        UdpPacketHeader h;
        h.seq = seq++;
        h.total = total;
        h.payload_len = static_cast<uint16_t>(chunk);

        std::vector<uint8_t> pkt(sizeof(UdpPacketHeader) + chunk);
        std::memcpy(pkt.data(), &h, sizeof(h));
        std::memcpy(pkt.data() + sizeof(h), bitstream.data() + offset, chunk);
        packets.push_back(std::move(pkt));
        offset += chunk;
    }
    return packets;
}

std::vector<uint8_t> reassemble_packets(const std::vector<std::vector<uint8_t>>& packets) {
    if (packets.empty()) return {};
    UdpPacketHeader first;
    std::memcpy(&first, packets[0].data(), sizeof(first));
    std::vector<uint8_t> out;
    out.reserve(first.total * UDP_PAYLOAD_MAX);

    for (const auto& pkt : packets) {
        if (pkt.size() < sizeof(UdpPacketHeader)) continue;
        UdpPacketHeader h;
        std::memcpy(&h, pkt.data(), sizeof(h));
        if (h.magic != 0x55445031) continue;
        out.insert(out.end(),
                   pkt.begin() + sizeof(UdpPacketHeader),
                   pkt.begin() + sizeof(UdpPacketHeader) + h.payload_len);
    }
    return out;
}
