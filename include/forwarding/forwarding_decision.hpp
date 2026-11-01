#ifndef FORWARDING_DECISION_HPP
#define FORWARDING_DECISION_HPP

#include <string>
#include <cstdint>

#include "packet/packet.hpp"
#include "routing/route_table.hpp"
#include "network/network_interface.hpp"

enum class ForwardingStatus {
    Ready,
    NoRoute,
    InterfaceUnavailable
};

struct ForwardingDecision {
    ForwardingStatus status;

    uint32_t nextHop;
    std::string interfaceName;
    std::string message;
};

class ForwardingDecisionEngine {
public:
    ForwardingDecision evaluate(
        const Packet& packet,
        const RouteTable& routingTable,
        InterfaceManager& interfaceManager
    );
};

#endif