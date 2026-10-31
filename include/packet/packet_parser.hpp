#ifndef PACKET_PARSER_HPP
#define PACKET_PARSER_HPP

#include "packet/packet.hpp"
#include <vector>

class PacketParser {

public:
    bool parseIPv4Packet(
        const std::vector<uint8_t>& rawData,
        Packet& packet
    );

};

#endif