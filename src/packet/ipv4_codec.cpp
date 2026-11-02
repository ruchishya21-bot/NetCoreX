#include "packet/ipv4_codec.hpp"
#include "packet/packet_parser.hpp"

#include <array>

namespace {

void write16(
    std::vector<uint8_t>& data,
    std::size_t position,
    uint16_t value
) {
    data[position] = static_cast<uint8_t>(value >> 8);
    data[position + 1] = static_cast<uint8_t>(value);
}

uint16_t checksum(
    const std::vector<uint8_t>& data,
    uint32_t initialSum = 0
) {
    uint32_t sum = initialSum;

    for (std::size_t i = 0; i + 1 < data.size(); i += 2) {
        sum +=
            (static_cast<uint16_t>(data[i]) << 8) |
            data[i + 1];
    }

    if (data.size() % 2 != 0) {
        sum += static_cast<uint16_t>(data.back()) << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return static_cast<uint16_t>(~sum);
}

uint32_t pseudoHeaderSum(
    uint32_t sourceIP,
    uint32_t destinationIP,
    uint8_t protocol,
    uint16_t length
) {
    return
        (sourceIP >> 16) +
        (sourceIP & 0xFFFF) +
        (destinationIP >> 16) +
        (destinationIP & 0xFFFF) +
        protocol +
        length;
}

}

bool IPv4PacketCodec::serialize(
    const Packet& packet,
    std::vector<uint8_t>& rawData
) {

    rawData.clear();

    std::vector<uint8_t> transport;

    // ==========================================
    // TCP HEADER
    // ==========================================

    if (packet.protocol == 6) {

        transport.resize(20, 0);

        write16(transport, 0, packet.sourcePort);
        write16(transport, 2, packet.destinationPort);

        // Sequence and acknowledgment numbers remain zero.
        transport[12] = 0x50;
        transport[13] = packet.tcpFlags;

        write16(transport, 14, 65535);

        transport.insert(
            transport.end(),
            packet.payload.begin(),
            packet.payload.end()
        );

        uint32_t pseudoSum = pseudoHeaderSum(
            packet.sourceIP,
            packet.destinationIP,
            6,
            static_cast<uint16_t>(transport.size())
        );

        write16(
            transport,
            16,
            checksum(transport, pseudoSum)
        );
    }

    // ==========================================
    // UDP HEADER
    // ==========================================

    else if (packet.protocol == 17) {

        transport.resize(8, 0);

        write16(transport, 0, packet.sourcePort);
        write16(transport, 2, packet.destinationPort);

        write16(
            transport,
            4,
            static_cast<uint16_t>(8 + packet.payload.size())
        );

        // UDP checksum zero is permitted for IPv4.

        transport.insert(
            transport.end(),
            packet.payload.begin(),
            packet.payload.end()
        );
    }

    // ==========================================
    // OTHER PROTOCOLS
    // ==========================================

    else {

        transport = packet.payload;
    }

    const std::size_t totalLength = 20 + transport.size();

    // Ethernet MTU is 1500 bytes.
    if (totalLength > 1500 || totalLength > 65535) {
        return false;
    }

    // ==========================================
    // IPV4 HEADER
    // ==========================================

    rawData.resize(20, 0);

    rawData[0] = 0x45;
    rawData[1] = 0;

    write16(
        rawData,
        2,
        static_cast<uint16_t>(totalLength)
    );

    write16(
        rawData,
        4,
        static_cast<uint16_t>(packet.packetId)
    );

    write16(rawData, 6, 0);

    rawData[8] = packet.ttl;
    rawData[9] = packet.protocol;

    rawData[10] = 0;
    rawData[11] = 0;

    rawData[12] = static_cast<uint8_t>(packet.sourceIP >> 24);
    rawData[13] = static_cast<uint8_t>(packet.sourceIP >> 16);
    rawData[14] = static_cast<uint8_t>(packet.sourceIP >> 8);
    rawData[15] = static_cast<uint8_t>(packet.sourceIP);

    rawData[16] = static_cast<uint8_t>(packet.destinationIP >> 24);
    rawData[17] = static_cast<uint8_t>(packet.destinationIP >> 16);
    rawData[18] = static_cast<uint8_t>(packet.destinationIP >> 8);
    rawData[19] = static_cast<uint8_t>(packet.destinationIP);

    write16(rawData, 10, checksum(rawData));

    rawData.insert(
        rawData.end(),
        transport.begin(),
        transport.end()
    );

    return true;
}

bool IPv4PacketCodec::deserialize(
    const std::vector<uint8_t>& rawData,
    Packet& packet
) {

    PacketParser parser;

    if (!parser.parseIPv4Packet(rawData, packet)) {
        return false;
    }

    packet.packetId =
        (static_cast<uint32_t>(rawData[4]) << 8) |
        rawData[5];

    return true;
}