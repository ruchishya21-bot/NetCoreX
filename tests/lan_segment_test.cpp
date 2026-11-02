#include <cassert>
#include <iostream>

#include "network/lan_segment.hpp"

void testDeviceRegistration() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");
    assert(mac.has_value());

    LanSegment lan;

    assert(lan.attachDevice(*mac, [](const EthernetFrame&) {}));
    assert(lan.hasDevice(*mac));
    assert(lan.getDeviceCount() == 1);

    std::cout << "PASS: Device registration\n";
}

void testDuplicateRegistration() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");
    assert(mac.has_value());

    LanSegment lan;

    assert(lan.attachDevice(*mac, [](const EthernetFrame&) {}));

    assert(!lan.attachDevice(
        *mac,
        [](const EthernetFrame&) {}
    ));

    assert(lan.getDeviceCount() == 1);

    std::cout << "PASS: Duplicate MAC rejection\n";
}

void testBroadcastDelivery() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:01");
    auto macC = MacAddress::parse("AA:BB:CC:DD:EE:02");

    assert(macA && macB && macC);

    LanSegment lan;

    int receivedB = 0;
    int receivedC = 0;

    lan.attachDevice(*macA, [](const EthernetFrame&) {});

    lan.attachDevice(*macB, [&](const EthernetFrame&) {
        receivedB++;
    });

    lan.attachDevice(*macC, [&](const EthernetFrame&) {
        receivedC++;
    });

    EthernetFrame frame(
        *macA,
        EthernetFrame::broadcastMAC(),
        EtherType::ARP,
        {1, 2, 3}
    );

    std::size_t delivered = lan.transmit(frame);

    assert(delivered == 2);
    assert(receivedB == 1);
    assert(receivedC == 1);

    std::cout << "PASS: Broadcast delivery\n";
}

void testUnicastDelivery() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:01");
    auto macC = MacAddress::parse("AA:BB:CC:DD:EE:02");

    assert(macA && macB && macC);

    LanSegment lan;

    int receivedB = 0;
    int receivedC = 0;

    lan.attachDevice(*macA, [](const EthernetFrame&) {});

    lan.attachDevice(*macB, [&](const EthernetFrame&) {
        receivedB++;
    });

    lan.attachDevice(*macC, [&](const EthernetFrame&) {
        receivedC++;
    });

    EthernetFrame frame(
        *macA,
        *macB,
        EtherType::IPv4,
        {10, 20, 30}
    );

    std::size_t delivered = lan.transmit(frame);

    assert(delivered == 1);
    assert(receivedB == 1);
    assert(receivedC == 0);

    std::cout << "PASS: Unicast delivery\n";
}

void testUnknownDestination() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:01");
    auto macC = MacAddress::parse("AA:BB:CC:DD:EE:02");

    assert(macA && macB && macC);

    LanSegment lan;

    lan.attachDevice(*macA, [](const EthernetFrame&) {});
    lan.attachDevice(*macB, [](const EthernetFrame&) {});

    EthernetFrame frame(
        *macA,
        *macC,
        EtherType::IPv4,
        {1, 2, 3}
    );

    assert(lan.transmit(frame) == 0);

    std::cout << "PASS: Unknown destination handling\n";
}

void testDeviceRemoval() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");
    assert(mac.has_value());

    LanSegment lan;

    lan.attachDevice(*mac, [](const EthernetFrame&) {});

    assert(lan.detachDevice(*mac));
    assert(!lan.hasDevice(*mac));
    assert(lan.getDeviceCount() == 0);

    std::cout << "PASS: Device removal\n";
}

int main() {

    testDeviceRegistration();
    testDuplicateRegistration();
    testBroadcastDelivery();
    testUnicastDelivery();
    testUnknownDestination();
    testDeviceRemoval();

    std::cout << "\nAll LAN segment tests passed.\n";

    return 0;
}