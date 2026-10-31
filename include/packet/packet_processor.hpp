#ifndef PACKET_PROCESSOR_HPP
#define PACKET_PROCESSOR_HPP

#include "packet/packet.hpp"
#include <string>

class PacketProcessor {

public:
    std::string identifyProtocol(const Packet& packet);
    void processPacket(const Packet& packet);

};

#endif