#ifndef FORWARDING_ENGINE_HPP
#define FORWARDING_ENGINE_HPP

#include "packet/packet.hpp"
#include "routing/route_table.hpp"
#include "network/network_interface.hpp"

class ForwardingEngine {

public:
    void forwardPacket(
        const Packet& packet,
        const RouteTable& table,
        InterfaceManager& manager
    );

};

#endif