#include "packet/packet_parser.hpp"

static uint16_t read16(
    const std::vector<uint8_t>& data,
    size_t pos
) {
    return (static_cast<uint16_t>(data[pos]) << 8) |
           static_cast<uint16_t>(data[pos + 1]);
}

static uint32_t read32(
    const std::vector<uint8_t>& data,
    size_t pos
) {
    return (static_cast<uint32_t>(data[pos]) << 24) |
           (static_cast<uint32_t>(data[pos + 1]) << 16) |
           (static_cast<uint32_t>(data[pos + 2]) << 8) |
           static_cast<uint32_t>(data[pos + 3]);
}

bool PacketParser::parseIPv4Packet(
    const std::vector<uint8_t>& rawData,
    Packet& packet
) {
    if (rawData.size() < 20) {
        return false;
    }

    uint8_t version = rawData[0] >> 4;
    uint8_t headerLength = (rawData[0] & 0x0F) * 4;

    if (version != 4 || headerLength < 20) {
        return false;
    }

    if (rawData.size() < headerLength) {
        return false;
    }

    uint16_t totalLength = read16(rawData, 2);

    if (totalLength < headerLength ||
        totalLength > rawData.size()) {
        return false;
    }

    packet.ttl = rawData[8];
    packet.protocol = rawData[9];
    packet.sourceIP = read32(rawData, 12);
    packet.destinationIP = read32(rawData, 16);

    // Reset transport information before parsing.
    packet.sourcePort = 0;
    packet.destinationPort = 0;
    packet.tcpFlags = 0;

    size_t transportOffset = headerLength;

    if (packet.protocol == 6) {

        // TCP
        if (totalLength < transportOffset + 20) {
            return false;
        }

        packet.sourcePort = read16(
            rawData,
            transportOffset
        );

        packet.destinationPort = read16(
            rawData,
            transportOffset + 2
        );

        uint8_t tcpHeaderLength =
            (rawData[transportOffset + 12] >> 4) * 4;

        if (tcpHeaderLength < 20 ||
            totalLength < transportOffset + tcpHeaderLength) {
            return false;
        }

        // Extract TCP control flags.
        packet.tcpFlags = rawData[transportOffset + 13];

        transportOffset += tcpHeaderLength;
    }

    else if (packet.protocol == 17) {

        // UDP
        if (totalLength < transportOffset + 8) {
            return false;
        }

        packet.sourcePort = read16(
            rawData,
            transportOffset
        );

        packet.destinationPort = read16(
            rawData,
            transportOffset + 2
        );

        uint16_t udpLength = read16(
            rawData,
            transportOffset + 4
        );

        if (udpLength < 8 ||
            udpLength > totalLength - transportOffset) {
            return false;
        }

        totalLength = transportOffset + udpLength;

        transportOffset += 8;
    }

    else {

        packet.sourcePort = 0;
        packet.destinationPort = 0;
        packet.tcpFlags = 0;
    }

    packet.payload.assign(
        rawData.begin() + transportOffset,
        rawData.begin() + totalLength
    );

    return true;
}