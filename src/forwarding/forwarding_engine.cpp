#include "forwarding/forwarding_engine.hpp"
#include "network/ip_address.hpp"
#include <iostream>

void ForwardingEngine::forwardPacket(
    const Packet& packet,
    const RouteTable& table,
    InterfaceManager& manager
) {

    std::cout << "\n========== FORWARDING ENGINE ==========\n";

    const Route* route = table.findRoute(packet.destinationIP);

    if (route == nullptr) {
        std::cout << "Action: DROP PACKET\n";
        std::cout << "Reason: No matching route\n";
        return;
    }

    NetworkInterface* interface =
        manager.findInterface(route->interfaceName);

    if (interface == nullptr) {
        std::cout << "Action: DROP PACKET\n";
        std::cout << "Reason: Interface not found\n";
        return;
    }

    if (!interface->isUp) {
        std::cout << "Action: DROP PACKET\n";
        std::cout << "Reason: Interface is DOWN\n";
        return;
    }

    interface->packetsForwarded++;
    interface->bytesForwarded += packet.payload.size();

    std::cout << "Packet ID: " << packet.packetId << "\n";

    std::cout << "Destination: "
              << formatIP(packet.destinationIP) << "\n";

    std::cout << "Next Hop: "
              << formatIP(route->nextHop) << "\n";

    std::cout << "Output Interface: "
              << interface->name << "\n";

    std::cout << "Action: FORWARD PACKET\n";

    std::cout << "Status: Forwarding successful\n";

    std::cout << "========================================\n";
}