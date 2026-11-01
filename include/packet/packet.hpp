#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <vector>

struct Packet {
    uint32_t packetId{};
    uint32_t sourceIP{};
    uint32_t destinationIP{};

    uint16_t sourcePort{};
    uint16_t destinationPort{};

    uint8_t protocol{};
    uint8_t tcpFlags{};
    uint8_t ttl{64};

    std::vector<uint8_t> payload;
};

#endif