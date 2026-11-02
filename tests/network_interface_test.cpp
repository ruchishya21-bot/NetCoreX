#include <cassert>
#include <iostream>

#include "network/network_interface.hpp"

void testAutomaticMacGeneration() {

    InterfaceManager manager;

    manager.addInterface(
        "eth0",
        0xC0A80101
    );

    auto* interface = manager.findInterface("eth0");

    assert(interface != nullptr);

    auto expected = MacAddress::parse(
        "02:00:C0:A8:01:01"
    );

    assert(expected.has_value());

    assert(interface->macAddress == *expected);

    std::cout << "PASS: Automatic MAC generation\n";
}

void testExplicitMacAssignment() {

    InterfaceManager manager;

    auto mac = MacAddress::parse(
        "AA:BB:CC:DD:EE:FF"
    );

    assert(mac.has_value());

    manager.addInterface(
        "eth1",
        0x0A000001,
        *mac
    );

    auto* interface = manager.findInterface("eth1");

    assert(interface != nullptr);
    assert(interface->macAddress == *mac);

    std::cout << "PASS: Explicit MAC assignment\n";
}

void testFindInterfaceByMAC() {

    InterfaceManager manager;

    auto mac = MacAddress::parse(
        "AA:BB:CC:DD:EE:FF"
    );

    assert(mac.has_value());

    manager.addInterface(
        "eth1",
        0x0A000001,
        *mac
    );

    auto* interface = manager.findInterfaceByMAC(*mac);

    assert(interface != nullptr);
    assert(interface->name == "eth1");

    std::cout << "PASS: Interface lookup by MAC\n";
}

void testDifferentIPsGenerateDifferentMACs() {

    InterfaceManager manager;

    manager.addInterface("eth0", 0x0A000001);
    manager.addInterface("eth1", 0x0A000002);

    auto* first = manager.findInterface("eth0");
    auto* second = manager.findInterface("eth1");

    assert(first != nullptr);
    assert(second != nullptr);

    assert(first->macAddress != second->macAddress);

    std::cout << "PASS: Unique generated MAC addresses\n";
}

int main() {

    testAutomaticMacGeneration();
    testExplicitMacAssignment();
    testFindInterfaceByMAC();
    testDifferentIPsGenerateDifferentMACs();

    std::cout << "\nAll network interface tests passed.\n";

    return 0;
}