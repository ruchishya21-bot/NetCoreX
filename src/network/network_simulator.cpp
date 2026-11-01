#include "network/network_simulator.hpp"
#include "forwarding/forwarding_decision.hpp"

#include <iostream>
#include <unordered_set>

void NetworkSimulator::addRouter(Router& router) {
    routers.push_back(&router);
}

ForwardingResult NetworkSimulator::simulate(
    Packet& packet,
    Router& sourceRouter
) {
    // Clear any ICMP error from the previous packet.
    lastIcmpError.reset();

    Router* currentRouter = &sourceRouter;
    ForwardingDecisionEngine decisionEngine;

    std::unordered_set<std::string> visited;

    constexpr int MAX_HOPS = 16;
    int hops = 0;

    std::cout << "\n========== PACKET FORWARDING ==========\n";
    std::cout << "Starting router: " << currentRouter->getName() << "\n";
    std::cout << "Initial TTL: "
              << static_cast<int>(packet.ttl) << "\n";

    while (hops < MAX_HOPS) {

        // Check for routing loops.
        if (!visited.insert(currentRouter->getName()).second) {
            std::cout << "DROP: Routing loop detected.\n";
            return ForwardingResult::RoutingLoop;
        }

        // Apply ACL rules if access control is enabled.
if (currentRouter->isAccessControlEnabled()) {

    ACLDecision aclDecision =
        currentRouter->getAccessControlList().evaluate(packet);

    if (aclDecision.action == ACLAction::Deny) {

        std::cout << "DROP: ACL denied packet at "
                  << currentRouter->getName() << "\n";

        std::cout << aclDecision.message << "\n";

        return ForwardingResult::AclDenied;
    }
}

        // Check TTL expiration and generate ICMP error.
        if (packet.ttl <= 1) {

            lastIcmpError = icmpHandler.createTtlExpiredError(
                packet,
                currentRouter->getName()
            );

            std::cout << "DROP: IPv4 TTL expired at "
                      << currentRouter->getName() << "\n";

            std::cout << lastIcmpError->message << "\n";

            return ForwardingResult::TtlExpired;
        }

        // Evaluate the packet's forwarding decision.
ForwardingDecision decision = decisionEngine.evaluate(
    packet,
    currentRouter->getRoutingTable(),
    currentRouter->getInterfaceManager()
);

// Handle missing routes.
if (decision.status == ForwardingStatus::NoRoute) {

    lastIcmpError =
        icmpHandler.createDestinationUnreachableError(
            packet,
            currentRouter->getName()
        );

    std::cout << "DROP: No matching route at "
              << currentRouter->getName() << "\n";

    std::cout << lastIcmpError->message << "\n";

    return ForwardingResult::NoRoute;
}

// Handle unavailable interfaces.
if (decision.status == ForwardingStatus::InterfaceUnavailable) {

    std::cout << "DROP: Outgoing interface unavailable at "
              << currentRouter->getName() << "\n";

    return ForwardingResult::InterfaceDown;
}

                   // Retrieve the selected outgoing interface.
        auto* outgoingInterface =
            currentRouter->getInterfaceManager()
                         .findInterface(decision.interfaceName);

        // Decrement TTL once per forwarding hop.
        packet.ttl--;

        outgoingInterface->packetsForwarded++;
        outgoingInterface->bytesForwarded += packet.payload.size();

        std::cout << currentRouter->getName()
                  << " -> interface " << outgoingInterface->name
                  << " | TTL: "
                  << static_cast<int>(packet.ttl)
                  << "\n";
        // Destination network reached.
        if (decision.nextHop == 0) {

            std::cout << "PACKET DELIVERED\n";
            std::cout << "Total hops: " << hops + 1 << "\n";
            std::cout << "Remaining TTL: "
                      << static_cast<int>(packet.ttl) << "\n";
            std::cout << "=======================================\n";

            return ForwardingResult::Delivered;
        }

        // Locate the next-hop router.
        Router* nextRouter = nullptr;

        for (Router* candidate : routers) {

            if (candidate == currentRouter) {
                continue;
            }

            if (candidate->getInterfaceManager()
                         .findInterfaceByIP(decision.nextHop) != nullptr) {

                nextRouter = candidate;
                break;
            }
        }

        if (nextRouter == nullptr) {

            std::cout << "DROP: Next-hop router is unreachable.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        currentRouter = nextRouter;
        hops++;
    }

    std::cout << "DROP: Maximum hop limit exceeded.\n";

    return ForwardingResult::HopLimitExceeded;
}


// Return the most recently generated ICMP error.
const std::optional<IcmpError>&
NetworkSimulator::getLastIcmpError() const {

    return lastIcmpError;
}