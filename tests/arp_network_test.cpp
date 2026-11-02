#include <cassert>
#include <iostream>

#include "network/arp_network.hpp"

void testDeviceRegistration() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA && macB);

    ArpNetwork network;

    assert(network.addDevice(0xC0A8010A, *macA));
    assert(network.addDevice(0xC0A80114, *macB));

    assert(network.getDeviceCount() == 2);

    std::cout << "PASS: Device registration\n";
}

void testDuplicateIP() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA && macB);

    ArpNetwork network;

    assert(network.addDevice(0xC0A8010A, *macA));

    assert(!network.addDevice(0xC0A8010A, *macB));

    std::cout << "PASS: Duplicate IP rejection\n";
}

void testDuplicateMAC() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac);

    ArpNetwork network;

    assert(network.addDevice(0xC0A8010A, *mac));

    assert(!network.addDevice(0xC0A80114, *mac));

    std::cout << "PASS: Duplicate MAC rejection\n";
}

void testArpResolutionAcrossLan() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA && macB);

    ArpNetwork network;

    constexpr uint32_t deviceA = 0xC0A8010A;
    constexpr uint32_t deviceB = 0xC0A80114;

    assert(network.addDevice(deviceA, *macA));
    assert(network.addDevice(deviceB, *macB));

    // Initially, A does not know B's MAC.
    assert(!network.resolve(deviceA, deviceB).has_value());

    // A broadcasts an ARP request.
    assert(network.requestResolution(deviceA, deviceB));

    // B replies through the Ethernet LAN.
    // A learns B's MAC automatically.
    auto resolved = network.resolve(deviceA, deviceB);

    assert(resolved.has_value());
    assert(*resolved == *macB);

    std::cout << "PASS: ARP resolution across LAN\n";
}

void testUnknownTarget() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");

    assert(macA);

    ArpNetwork network;

    constexpr uint32_t deviceA = 0xC0A8010A;
    constexpr uint32_t unknown = 0xC0A80199;

    assert(network.addDevice(deviceA, *macA));

    assert(network.requestResolution(deviceA, unknown));

    assert(!network.resolve(deviceA, unknown).has_value());

    std::cout << "PASS: Unknown target handling\n";
}

void testDeviceRemoval() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac);

    ArpNetwork network;

    constexpr uint32_t deviceA = 0xC0A8010A;

    assert(network.addDevice(deviceA, *mac));

    assert(network.removeDevice(deviceA));

    assert(network.getDeviceCount() == 0);

    assert(!network.removeDevice(deviceA));

    std::cout << "PASS: Device removal\n";
}

int main() {

    testDeviceRegistration();
    testDuplicateIP();
    testDuplicateMAC();
    testArpResolutionAcrossLan();
    testUnknownTarget();
    testDeviceRemoval();

    std::cout << "\nAll ARP network integration tests passed.\n";

    return 0;
}