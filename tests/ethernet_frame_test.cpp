#include <cassert>
#include <iostream>
#include <vector>

#include "network/ethernet_frame.hpp"

void testEthernetFrameCreation() {

    auto source = MacAddress::parse("00:11:22:33:44:55");
    auto destination = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(source.has_value());
    assert(destination.has_value());

    std::vector<uint8_t> payload = {1, 2, 3, 4};

    EthernetFrame frame(
        *source,
        *destination,
        EtherType::IPv4,
        payload
    );

    assert(frame.getSourceMAC() == *source);
    assert(frame.getDestinationMAC() == *destination);
    assert(frame.getEtherType() == EtherType::IPv4);
    assert(frame.getPayload() == payload);

    std::cout << "PASS: Ethernet frame creation\n";
}

void testBroadcastAddress() {

    MacAddress broadcast = EthernetFrame::broadcastMAC();

    auto expected = MacAddress::parse("FF:FF:FF:FF:FF:FF");

    assert(expected.has_value());
    assert(broadcast == *expected);

    std::cout << "PASS: Broadcast MAC address\n";
}

void testBroadcastFrame() {

    auto source = MacAddress::parse("00:11:22:33:44:55");

    assert(source.has_value());

    EthernetFrame frame(
        *source,
        EthernetFrame::broadcastMAC(),
        EtherType::ARP,
        {1, 2, 3}
    );

    assert(frame.isBroadcast());
    assert(frame.isValid());

    std::cout << "PASS: Ethernet broadcast frame\n";
}

void testUnicastFrame() {

    auto source = MacAddress::parse("00:11:22:33:44:55");
    auto destination = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(source.has_value());
    assert(destination.has_value());

    EthernetFrame frame(
        *source,
        *destination,
        EtherType::IPv4,
        {10, 20, 30}
    );

    assert(!frame.isBroadcast());
    assert(frame.isValid());

    std::cout << "PASS: Ethernet unicast frame\n";
}

void testPayloadLimit() {

    auto source = MacAddress::parse("00:11:22:33:44:55");
    auto destination = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(source.has_value());
    assert(destination.has_value());

    std::vector<uint8_t> payload(
        EthernetFrame::MAX_PAYLOAD_SIZE + 1,
        0
    );

    bool rejected = false;

    try {
        EthernetFrame frame(
            *source,
            *destination,
            EtherType::IPv4,
            payload
        );
    }
    catch (const std::invalid_argument&) {
        rejected = true;
    }

    assert(rejected);

    std::cout << "PASS: Ethernet payload size validation\n";
}

int main() {

    testEthernetFrameCreation();
    testBroadcastAddress();
    testBroadcastFrame();
    testUnicastFrame();
    testPayloadLimit();

    std::cout << "\nAll Ethernet frame tests passed.\n";

    return 0;
}