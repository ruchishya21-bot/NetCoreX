#include <iostream>
#include <cassert>

#include "forwarding/forwarding_decision.hpp"

void testSuccessfulForwarding() {
    RouteTable routingTable;
    InterfaceManager interfaceManager;
    Packet packet{};

    packet.destinationIP = 0xC0A80205;

    interfaceManager.addInterface("eth0", 0xC0A80101);

    routingTable.addRoute(
        0xC0A80200,
        24,
        0xC0A80102,
        "eth0"
    );

    ForwardingDecisionEngine engine;

    ForwardingDecision decision =
        engine.evaluate(packet, routingTable, interfaceManager);

    assert(decision.status == ForwardingStatus::Ready);
    assert(decision.nextHop == 0xC0A80102);
    assert(decision.interfaceName == "eth0");

    std::cout << "PASS: Successful forwarding decision\n";
}

void testMissingRoute() {
    RouteTable routingTable;
    InterfaceManager interfaceManager;
    Packet packet{};

    packet.destinationIP = 0xC0A80205;

    ForwardingDecisionEngine engine;

    ForwardingDecision decision =
        engine.evaluate(packet, routingTable, interfaceManager);

    assert(decision.status == ForwardingStatus::NoRoute);

    std::cout << "PASS: Missing route detection\n";
}

void testInterfaceUnavailable() {
    RouteTable routingTable;
    InterfaceManager interfaceManager;
    Packet packet{};

    packet.destinationIP = 0xC0A80205;

    interfaceManager.addInterface("eth0", 0xC0A80101);

    // Simulate an interface failure.
    interfaceManager.findInterface("eth0")->isUp = false;

    routingTable.addRoute(
        0xC0A80200,
        24,
        0xC0A80102,
        "eth0"
    );

    ForwardingDecisionEngine engine;

    ForwardingDecision decision =
        engine.evaluate(packet, routingTable, interfaceManager);

    assert(decision.status == ForwardingStatus::InterfaceUnavailable);

    std::cout << "PASS: Interface failure detection\n";
}

int main() {
    testSuccessfulForwarding();
    testMissingRoute();
    testInterfaceUnavailable();

    std::cout << "\nAll forwarding decision tests passed!\n";

    return 0;
}