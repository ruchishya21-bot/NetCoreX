#include <iostream>
#include <vector>

#include "packet/packet.hpp"
#include "packet/packet_parser.hpp"

int main() {

    PacketParser parser;
    Packet packet{};

    int passed = 0;
    int failed = 0;

    // TEST 1: Valid TCP SYN packet
    std::vector<uint8_t> tcpPacket = {
        0x45, 0x00, 0x00, 0x2D,
        0x00, 0x01, 0x40, 0x00,
        0x40, 0x06, 0x00, 0x00,
        0xC0, 0xA8, 0x01, 0x01,
        0xC0, 0xA8, 0x01, 0x02,
        0x13, 0x88, 0x1F, 0x90,
        0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00,
        0x50, 0x02, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x00,
        0x48, 0x65, 0x6C, 0x6C, 0x6F
    };

    if (parser.parseIPv4Packet(tcpPacket, packet) &&
        packet.sourcePort == 5000 &&
        packet.destinationPort == 8080 &&
        packet.tcpFlags == 0x02 &&
        packet.payload.size() == 5) {

        std::cout << "[PASS] Valid TCP SYN packet\n";
        passed++;

    } else {
        std::cout << "[FAIL] Valid TCP SYN packet\n";
        failed++;
    }

    // TEST 2: Packet smaller than IPv4 header
    std::vector<uint8_t> shortPacket = {
        0x45, 0x00, 0x00
    };

    if (!parser.parseIPv4Packet(shortPacket, packet)) {
        std::cout << "[PASS] Reject incomplete packet\n";
        passed++;
    } else {
        std::cout << "[FAIL] Reject incomplete packet\n";
        failed++;
    }

    // TEST 3: Invalid IP version
    std::vector<uint8_t> invalidVersion = tcpPacket;
    invalidVersion[0] = 0x65;

    if (!parser.parseIPv4Packet(invalidVersion, packet)) {
        std::cout << "[PASS] Reject invalid IP version\n";
        passed++;
    } else {
        std::cout << "[FAIL] Reject invalid IP version\n";
        failed++;
    }

    // TEST 4: Invalid total length
    std::vector<uint8_t> invalidLength = tcpPacket;
    invalidLength[2] = 0x00;
    invalidLength[3] = 0x10;

    if (!parser.parseIPv4Packet(invalidLength, packet)) {
        std::cout << "[PASS] Reject invalid packet length\n";
        passed++;
    } else {
        std::cout << "[FAIL] Reject invalid packet length\n";
        failed++;
    }

    // TEST 5: Valid UDP packet
    std::vector<uint8_t> udpPacket = {
        0x45, 0x00, 0x00, 0x20,
        0x00, 0x02, 0x40, 0x00,
        0x40, 0x11, 0x00, 0x00,
        0xC0, 0xA8, 0x01, 0x01,
        0xC0, 0xA8, 0x01, 0x02,

        0x13, 0x88, 0x00, 0x35,
        0x00, 0x0C, 0x00, 0x00,

        0x44, 0x41, 0x54, 0x41
    };

    if (parser.parseIPv4Packet(udpPacket, packet) &&
        packet.protocol == 17 &&
        packet.sourcePort == 5000 &&
        packet.destinationPort == 53 &&
        packet.tcpFlags == 0 &&
        packet.payload.size() == 4) {

        std::cout << "[PASS] Valid UDP packet\n";
        passed++;

    } else {
        std::cout << "[FAIL] Valid UDP packet\n";
        failed++;
    }

    std::cout << "\n========== TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "==================================\n";

    return failed == 0 ? 0 : 1;
}