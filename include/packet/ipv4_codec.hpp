#ifndef IPV4_CODEC_HPP
#define IPV4_CODEC_HPP

#include <cstdint>
#include <vector>

#include "packet/packet.hpp"

class IPv4PacketCodec {

public:

    static bool serialize(
        const Packet& packet,
        std::vector<uint8_t>& rawData
    );

    static bool deserialize(
        const std::vector<uint8_t>& rawData,
        Packet& packet
    );
};

#endif