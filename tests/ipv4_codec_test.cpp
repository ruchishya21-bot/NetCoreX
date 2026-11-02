#include <iostream>
#include <cassert>
#include <vector>

#include "packet/ipv4_codec.hpp"

void testTcpSerialization() {

    Packet original;

    original.packetId = 101;
    original.sourceIP = 0xC0A80101;
    original.destinationIP = 0xC0A80202;
    original.sourcePort = 50000;
    original.destinationPort = 443;
    original.protocol = 6;
    original.tcpFlags = 0x02;
    original.ttl = 64;
    original.payload = {10, 20, 30, 40};

    std::vector<uint8_t> rawData;

    assert(IPv4PacketCodec::serialize(original, rawData));

    Packet decoded;

    assert(IPv4PacketCodec::deserialize(rawData, decoded));

    assert(decoded.packetId == original.packetId);
    assert(decoded.sourceIP == original.sourceIP);
    assert(decoded.destinationIP == original.destinationIP);
    assert(decoded.sourcePort == original.sourcePort);
    assert(decoded.destinationPort == original.destinationPort);
    assert(decoded.protocol == original.protocol);
    assert(decoded.tcpFlags == original.tcpFlags);
    assert(decoded.ttl == original.ttl);
    assert(decoded.payload == original.payload);

    std::cout << "PASS: TCP serialization and deserialization\n";
}


void testUdpSerialization() {

    Packet original;

    original.packetId = 202;
    original.sourceIP = 0x0A000001;
    original.destinationIP = 0x0A000002;
    original.sourcePort = 40000;
    original.destinationPort = 53;
    original.protocol = 17;
    original.ttl = 128;
    original.payload = {1, 2, 3, 4, 5};

    std::vector<uint8_t> rawData;

    assert(IPv4PacketCodec::serialize(original, rawData));

    Packet decoded;

    assert(IPv4PacketCodec::deserialize(rawData, decoded));

    assert(decoded.sourceIP == original.sourceIP);
    assert(decoded.destinationIP == original.destinationIP);
    assert(decoded.sourcePort == original.sourcePort);
    assert(decoded.destinationPort == original.destinationPort);
    assert(decoded.protocol == original.protocol);
    assert(decoded.ttl == original.ttl);
    assert(decoded.payload == original.payload);

    std::cout << "PASS: UDP serialization and deserialization\n";
}


void testIcmpSerialization() {

    Packet original;

    original.packetId = 303;
    original.sourceIP = 0xC0A80101;
    original.destinationIP = 0xC0A80102;
    original.protocol = 1;
    original.ttl = 32;
    original.payload = {8, 0, 0, 0, 1, 2, 3, 4};

    std::vector<uint8_t> rawData;

    assert(IPv4PacketCodec::serialize(original, rawData));

    Packet decoded;

    assert(IPv4PacketCodec::deserialize(rawData, decoded));

    assert(decoded.sourceIP == original.sourceIP);
    assert(decoded.destinationIP == original.destinationIP);
    assert(decoded.protocol == original.protocol);
    assert(decoded.ttl == original.ttl);
    assert(decoded.payload == original.payload);

    std::cout << "PASS: ICMP serialization and deserialization\n";
}


void testIpv4HeaderFields() {

    Packet packet;

    packet.sourceIP = 0xC0A80101;
    packet.destinationIP = 0xC0A80202;
    packet.protocol = 17;
    packet.ttl = 64;
    packet.payload = {1, 2, 3};

    std::vector<uint8_t> rawData;

    assert(IPv4PacketCodec::serialize(packet, rawData));

    // Minimum IPv4 header size.
    assert(rawData.size() >= 20);

    // IPv4 version and header length.
    assert(rawData[0] == 0x45);

    // TTL.
    assert(rawData[8] == 64);

    // Protocol.
    assert(rawData[9] == 17);

    // Source IP.
    assert(rawData[12] == 192);
    assert(rawData[13] == 168);
    assert(rawData[14] == 1);
    assert(rawData[15] == 1);

    // Destination IP.
    assert(rawData[16] == 192);
    assert(rawData[17] == 168);
    assert(rawData[18] == 2);
    assert(rawData[19] == 2);

    std::cout << "PASS: IPv4 header field encoding\n";
}


void testEmptyPayload() {

    Packet original;

    original.sourceIP = 0x0A000001;
    original.destinationIP = 0x0A000002;
    original.protocol = 1;
    original.ttl = 64;

    std::vector<uint8_t> rawData;

    assert(IPv4PacketCodec::serialize(original, rawData));

    Packet decoded;

    assert(IPv4PacketCodec::deserialize(rawData, decoded));

    assert(decoded.payload.empty());

    std::cout << "PASS: Empty payload handling\n";
}


void testMalformedPacket() {

    std::vector<uint8_t> rawData = {
        0x45, 0x00, 0x00, 0x14
    };

    Packet decoded;

    assert(!IPv4PacketCodec::deserialize(rawData, decoded));

    std::cout << "PASS: Malformed IPv4 packet rejection\n";
}


void testOversizedPacket() {

    Packet packet;

    packet.sourceIP = 0x0A000001;
    packet.destinationIP = 0x0A000002;
    packet.protocol = 1;
    packet.ttl = 64;

    // Exceeds the codec's 1500-byte total packet limit.
    packet.payload.resize(1481, 0xAB);

    std::vector<uint8_t> rawData;

    assert(!IPv4PacketCodec::serialize(packet, rawData));

    std::cout << "PASS: Oversized packet rejection\n";
}


int main() {

    std::cout << "\n========== IPv4 CODEC TESTS ==========\n";

    testTcpSerialization();
    testUdpSerialization();
    testIcmpSerialization();
    testIpv4HeaderFields();
    testEmptyPayload();
    testMalformedPacket();
    testOversizedPacket();

    std::cout << "\nALL IPv4 CODEC TESTS PASSED\n";

    return 0;
}