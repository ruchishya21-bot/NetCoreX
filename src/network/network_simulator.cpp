#include "network/network_simulator.hpp"
#include "forwarding/forwarding_decision.hpp"

#include <iostream>
#include <unordered_set>

namespace {

constexpr uint8_t TCP_FIN = 0x01;
constexpr uint8_t TCP_SYN = 0x02;
constexpr uint8_t TCP_RST = 0x04;
constexpr uint8_t TCP_ACK = 0x10;

bool isInitialTcpSyn(const Packet& packet) {

    const bool syn = (packet.tcpFlags & TCP_SYN) != 0;
    const bool ack = (packet.tcpFlags & TCP_ACK) != 0;
    const bool fin = (packet.tcpFlags & TCP_FIN) != 0;
    const bool rst = (packet.tcpFlags & TCP_RST) != 0;

    return syn && !ack && !fin && !rst;
}

}

void NetworkSimulator::addRouter(Router& router) {
    routers.push_back(&router);
}

ForwardingResult NetworkSimulator::simulate(
    Packet& packet,
    Router& sourceRouter
) {

    lastIcmpError.reset();

    Router* currentRouter = &sourceRouter;

    ForwardingDecisionEngine decisionEngine;

    std::unordered_set<std::string> visited;

    constexpr int MAX_HOPS = 16;

    int hops = 0;

    std::cout << "\n========== PACKET FORWARDING ==========\n";

    std::cout << "Starting router: "
              << currentRouter->getName() << "\n";

    std::cout << "Initial TTL: "
              << static_cast<int>(packet.ttl) << "\n";

    while (hops < MAX_HOPS) {

        // Detect routing loops.
        if (!visited.insert(currentRouter->getName()).second) {

            std::cout << "DROP: Routing loop detected.\n";

            return ForwardingResult::RoutingLoop;
        }

        // ==========================================
        // STATEFUL FIREWALL
        // ==========================================

        if (currentRouter->isStatefulFirewallEnabled()) {

            if (packet.protocol == 6) {

                ConnectionTracker& tracker =
                    currentRouter->getConnectionTracker();

                ConnectionState existingState;

                const bool connectionExists =
                    tracker.getConnectionState(
                        packet,
                        existingState
                    );

                if (connectionExists) {

                    // Existing connection:
                    // Validate and update its TCP state.

                    ConnectionUpdateResult update =
                        tracker.processPacket(packet);

                    if (
                        update.status ==
                            ConnectionStatus::InvalidTransition ||
                        update.status ==
                            ConnectionStatus::UnsupportedProtocol
                    ) {

                        std::cout
                            << "STATEFUL FIREWALL: Packet rejected at "
                            << currentRouter->getName()
                            << "\n";

                        std::cout << update.message << "\n";

                        return ForwardingResult::StatefulFirewallDenied;
                    }

                    std::cout
                        << "STATEFUL FIREWALL: Existing connection accepted. "
                        << update.message << "\n";
                }

                else {

                    // An unknown TCP packet must begin with
                    // a valid initial SYN.

                    if (!isInitialTcpSyn(packet)) {

                        std::cout
                            << "STATEFUL FIREWALL: Unknown TCP packet rejected.\n";

                        return ForwardingResult::StatefulFirewallDenied;
                    }

                    // Apply ACL rules to new connections
                    // when ACL is enabled.

                    if (currentRouter->isAccessControlEnabled()) {

                        ACLDecision aclDecision =
                            currentRouter->getAccessControlList()
                                         .evaluate(packet);

                        if (aclDecision.action == ACLAction::Deny) {

                            std::cout
                                << "STATEFUL FIREWALL: New connection rejected by ACL.\n";

                            std::cout << aclDecision.message << "\n";

                            return ForwardingResult::AclDenied;
                        }
                    }

                    // Create a new tracked TCP connection.

                    ConnectionUpdateResult update =
                        tracker.processPacket(packet);

                    if (
                        update.status !=
                        ConnectionStatus::NewConnection
                    ) {

                        std::cout
                            << "STATEFUL FIREWALL: Unable to create connection.\n";

                        std::cout << update.message << "\n";

                        return ForwardingResult::StatefulFirewallDenied;
                    }

                    std::cout
                        << "STATEFUL FIREWALL: New TCP connection accepted.\n";
                }
            }

            else {

                // Non-TCP traffic continues to use
                // the existing ACL mechanism.

                if (currentRouter->isAccessControlEnabled()) {

                    ACLDecision aclDecision =
                        currentRouter->getAccessControlList()
                                     .evaluate(packet);

                    if (aclDecision.action == ACLAction::Deny) {

                        std::cout
                            << "DROP: ACL denied non-TCP packet at "
                            << currentRouter->getName() << "\n";

                        std::cout << aclDecision.message << "\n";

                        return ForwardingResult::AclDenied;
                    }
                }
            }
        }

        // ==========================================
        // STATELESS ACL
        // ==========================================

        else if (currentRouter->isAccessControlEnabled()) {

            ACLDecision aclDecision =
                currentRouter->getAccessControlList()
                             .evaluate(packet);

            if (aclDecision.action == ACLAction::Deny) {

                std::cout
                    << "DROP: ACL denied packet at "
                    << currentRouter->getName() << "\n";

                std::cout << aclDecision.message << "\n";

                return ForwardingResult::AclDenied;
            }
        }

        // ==========================================
        // TTL CHECK
        // ==========================================

        if (packet.ttl <= 1) {

            lastIcmpError =
                icmpHandler.createTtlExpiredError(
                    packet,
                    currentRouter->getName()
                );

            std::cout
                << "DROP: IPv4 TTL expired at "
                << currentRouter->getName() << "\n";

            std::cout << lastIcmpError->message << "\n";

            return ForwardingResult::TtlExpired;
        }

        // ==========================================
        // FORWARDING DECISION
        // ==========================================

        ForwardingDecision decision =
            decisionEngine.evaluate(
                packet,
                currentRouter->getRoutingTable(),
                currentRouter->getInterfaceManager()
            );

        if (decision.status == ForwardingStatus::NoRoute) {

            lastIcmpError =
                icmpHandler.createDestinationUnreachableError(
                    packet,
                    currentRouter->getName()
                );

            std::cout
                << "DROP: No matching route at "
                << currentRouter->getName() << "\n";

            std::cout << lastIcmpError->message << "\n";

            return ForwardingResult::NoRoute;
        }

        if (
            decision.status ==
            ForwardingStatus::InterfaceUnavailable
        ) {

            std::cout
                << "DROP: Outgoing interface unavailable at "
                << currentRouter->getName() << "\n";

            return ForwardingResult::InterfaceDown;
        }

        auto* outgoingInterface =
            currentRouter->getInterfaceManager()
                         .findInterface(decision.interfaceName);

        // Decrement TTL for this forwarding hop.
        packet.ttl--;

        outgoingInterface->packetsForwarded++;

        outgoingInterface->bytesForwarded +=
            packet.payload.size();

        std::cout
            << currentRouter->getName()
            << " -> interface "
            << outgoingInterface->name
            << " | TTL: "
            << static_cast<int>(packet.ttl)
            << "\n";

        // Destination network reached.
        if (decision.nextHop == 0) {

            std::cout << "PACKET DELIVERED\n";

            std::cout << "Total hops: "
                      << hops + 1 << "\n";

            std::cout << "Remaining TTL: "
                      << static_cast<int>(packet.ttl) << "\n";

            std::cout
                << "=======================================\n";

            return ForwardingResult::Delivered;
        }

        // Locate the next-hop router.
        Router* nextRouter = nullptr;

        for (Router* candidate : routers) {

            if (candidate == currentRouter) {
                continue;
            }

            if (
                candidate->getInterfaceManager()
                         .findInterfaceByIP(decision.nextHop) != nullptr
            ) {

                nextRouter = candidate;

                break;
            }
        }

        if (nextRouter == nullptr) {

            std::cout
                << "DROP: Next-hop router is unreachable.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        currentRouter = nextRouter;

        hops++;
    }

    std::cout
        << "DROP: Maximum hop limit exceeded.\n";

    return ForwardingResult::HopLimitExceeded;
}

const std::optional<IcmpError>&
NetworkSimulator::getLastIcmpError() const {

    return lastIcmpError;
}