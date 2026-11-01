#include <iostream>
#include <vector>

#include "packet/packet.hpp"
#include "packet/packet_parser.hpp"
#include "network/ip_address.hpp"
#include "network/router.hpp"
#include "network/network_simulator.hpp"
#include "routing/control_plane.hpp"

int main() {


// Create an IPv4 TCP packet.
std::vector<uint8_t> rawPacket = {
    0x45, 0x00, 0x00, 0x2D,
    0x00, 0x01, 0x40, 0x00,
    0x40, 0x06, 0x00, 0x00,
    0xC0, 0xA8, 0x01, 0x01,
    0xC0, 0xA8, 0x02, 0x02,

    0x13, 0x88, 0x1F, 0x90,
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00,
    0x50, 0x02, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00,

    0x48, 0x65, 0x6C, 0x6C, 0x6F
};

Packet packet{};
packet.packetId = 101;

PacketParser parser;

if (!parser.parseIPv4Packet(rawPacket, packet)) {
    std::cout << "Invalid packet\n";
    return 1;
}

std::cout << "========== NetCoreX ==========\n";
std::cout << "Source: " << formatIP(packet.sourceIP) << "\n";
std::cout << "Destination: " << formatIP(packet.destinationIP) << "\n";

// Create four routers.
Router r1("Router-1");
Router r2("Router-2");
Router r3("Router-3");
Router r4("Router-4");

// Router interfaces.
r1.addInterface("eth12", 0x0A000001);
r1.addInterface("eth13", 0x0A000002);

r2.addInterface("eth21", 0x0A000011);
r2.addInterface("eth24", 0x0A000012);

r3.addInterface("eth31", 0x0A000021);
r3.addInterface("eth34", 0x0A000022);

r4.addInterface("eth42", 0x0A000031);
r4.addInterface("eth43", 0x0A000032);
r4.addInterface("ethLAN", 0xC0A80201);

// Configure the control plane.
ControlPlane controlPlane;

controlPlane.registerRouter(r1);
controlPlane.registerRouter(r2);
controlPlane.registerRouter(r3);
controlPlane.registerRouter(r4);

// Primary path: Router-1 -> Router-2 -> Router-4.
controlPlane.addLink("Router-1", "eth12",
                     "Router-2", "eth21", 2);

controlPlane.addLink("Router-2", "eth24",
                     "Router-4", "eth42", 2);

// Alternate path: Router-1 -> Router-3 -> Router-4.
controlPlane.addLink("Router-1", "eth13",
                     "Router-3", "eth31", 5);

controlPlane.addLink("Router-3", "eth34",
                     "Router-4", "eth43", 5);

// Router-4 advertises the destination network.
controlPlane.advertiseNetwork(
    "Router-4",
    0xC0A80200,
    24,
    "ethLAN"
);

// Calculate and install dynamic routes.
controlPlane.installRoutes();

// Register routers with the packet simulator.
NetworkSimulator simulator;

simulator.addRouter(r1);
simulator.addRouter(r2);
simulator.addRouter(r3);
simulator.addRouter(r4);

// Keep an independent copy for the failover test.
Packet failoverPacket = packet;

// Test 1: Normal forwarding.
std::cout << "\n========== TEST 1: NORMAL FORWARDING ==========\n";

simulator.simulate(packet, r1);

// Test 2: Remove the primary path.
std::cout << "\n========== TEST 2: PRIMARY LINK FAILURE ==========\n";

controlPlane.removeLink("Router-2", "Router-4");

// Recalculate routes using the remaining topology.
controlPlane.installRoutes();

// Forward the same packet again.
simulator.simulate(failoverPacket, r1);

return 0;


}
