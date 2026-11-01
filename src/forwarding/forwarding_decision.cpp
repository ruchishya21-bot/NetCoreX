#include "forwarding/forwarding_decision.hpp"

ForwardingDecision ForwardingDecisionEngine::evaluate(
    const Packet& packet,
    const RouteTable& routingTable,
    InterfaceManager& interfaceManager
) {
    // Step 1: Find the best matching route.
    const auto* route =
        routingTable.findRoute(packet.destinationIP);

    if (route == nullptr) {
        return {
            ForwardingStatus::NoRoute,
            0,
            "",
            "No matching route found"
        };
    }

    // Step 2: Find the outgoing interface.
    const auto* interface =
        interfaceManager.findInterface(route->interfaceName);

    if (interface == nullptr || !interface->isUp) {
        return {
            ForwardingStatus::InterfaceUnavailable,
            route->nextHop,
            route->interfaceName,
            "Outgoing interface is unavailable"
        };
    }

    // Step 3: Return the forwarding decision.
    return {
        ForwardingStatus::Ready,
        route->nextHop,
        route->interfaceName,
        "Packet is ready for forwarding"
    };
}