#include "network/network_simulator.hpp"

#include <iostream>
#include <unordered_set>

void NetworkSimulator::addRouter(Router& router) {
    routers.push_back(&router);
}

void NetworkSimulator::simulate(Packet& packet, Router& sourceRouter) {
    Router* currentRouter = &sourceRouter;

    std::unordered_set<std::string> visited;

    constexpr int MAX_HOPS = 16;
    int hops = 0;

    std::cout << "\n========== PACKET FORWARDING ==========\n";
    std::cout << "Starting router: " << currentRouter->getName() << "\n";

    while (hops < MAX_HOPS) {

        if (!visited.insert(currentRouter->getName()).second) {
            std::cout << "DROP: Routing loop detected.\n";
            return;
        }

        const auto* route =
            currentRouter->getRoutingTable().findRoute(packet.destinationIP);

        if (route == nullptr) {
            std::cout << "DROP: No matching route at "
                      << currentRouter->getName() << "\n";
            return;
        }

        auto* outgoingInterface =
            currentRouter->getInterfaceManager()
                         .findInterface(route->interfaceName);

        if (outgoingInterface == nullptr || !outgoingInterface->isUp) {
            std::cout << "DROP: Outgoing interface unavailable at "
                      << currentRouter->getName() << "\n";
            return;
        }

        outgoingInterface->packetsForwarded++;
        outgoingInterface->bytesForwarded += packet.payload.size();

        std::cout << currentRouter->getName()
                  << " -> interface "
                  << outgoingInterface->name << "\n";

        // A zero next-hop indicates a directly connected destination.
        if (route->nextHop == 0) {
            std::cout << "PACKET DELIVERED\n";
            std::cout << "Total hops: " << hops + 1 << "\n";
            std::cout << "=======================================\n";
            return;
        }

        Router* nextRouter = nullptr;

        for (Router* candidate : routers) {
            if (candidate == currentRouter) {
                continue;
            }

            if (candidate->getInterfaceManager()
                         .findInterfaceByIP(route->nextHop) != nullptr) {
                nextRouter = candidate;
                break;
            }
        }

        if (nextRouter == nullptr) {
            std::cout << "DROP: Next-hop router is unreachable.\n";
            return;
        }

        currentRouter = nextRouter;
        hops++;
    }

    std::cout << "DROP: Maximum hop limit exceeded.\n";
}