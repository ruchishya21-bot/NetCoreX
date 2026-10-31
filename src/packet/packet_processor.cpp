#include "packet/packet_processor.hpp"
#include "network/ip_address.hpp"
#include <iostream>

std::string PacketProcessor::identifyProtocol(const Packet& packet) {

    if (packet.protocol == 6) {
        return "TCP";
    }

    if (packet.protocol == 17) {
        return "UDP";
    }

    if (packet.protocol == 1) {
        return "ICMP";
    }

    return "UNKNOWN";
}

void PacketProcessor::processPacket(const Packet& packet) {

    std::cout << "\n========== PACKET PROCESSING ==========\n";

    std::cout << "Packet ID: " << packet.packetId << "\n";
    std::cout << "Source: " << formatIP(packet.sourceIP) << "\n";
    std::cout << "Destination: " << formatIP(packet.destinationIP) << "\n";
    std::cout << "Protocol: " << identifyProtocol(packet) << "\n";
    std::cout << "Payload: " << packet.payload.size() << " bytes\n";

    std::cout << "Status: Packet processed successfully\n";
    std::cout << "=======================================\n";
}