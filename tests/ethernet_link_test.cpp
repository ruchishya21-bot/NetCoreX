#include "network/network_simulator.hpp"

#include <cassert>
#include <iostream>
#include <vector>

int main() {

    Router r1("Router-1");
    Router r2("Router-2");

    r1.addInterface("eth1", 0x0A000001);
    r2.addInterface("eth2", 0x0A000002);

    NetworkSimulator simulator;

    simulator.addRouter(r1);
    simulator.addRouter(r2);

    // Test 1: Establish Ethernet link.
    assert(simulator.connectInterfaces(r1, "eth1", r2, "eth2"));
    assert(simulator.getLinkCount() == 1);

    std::cout << "[PASS] Ethernet link established\n";

    // Retrieve interface MAC addresses.
    auto* interface1 =
        r1.getInterfaceManager().findInterface("eth1");

    auto* interface2 =
        r2.getInterfaceManager().findInterface("eth2");

    assert(interface1 != nullptr);
    assert(interface2 != nullptr);

    // Test 2: Broadcast transmission.
    EthernetFrame broadcastFrame(
        interface1->macAddress,
        EthernetFrame::broadcastMAC(),
        EtherType::IPv4,
        {0x45, 0x00, 0x00, 0x14}
    );

    assert(simulator.transmitFrame(r1, "eth1", broadcastFrame));

    auto received = simulator.takeReceivedFrames();

    assert(received.size() == 1);
    assert(received[0].router == &r2);
    assert(received[0].interfaceName == "eth2");

    std::cout << "[PASS] Ethernet broadcast delivery\n";

    // Test 3: Unicast transmission.
    EthernetFrame unicastFrame(
        interface1->macAddress,
        interface2->macAddress,
        EtherType::IPv4,
        {0x45, 0x00, 0x00, 0x14}
    );

    assert(simulator.transmitFrame(r1, "eth1", unicastFrame));

    received = simulator.takeReceivedFrames();

    assert(received.size() == 1);
    assert(received[0].router == &r2);

    std::cout << "[PASS] Ethernet unicast delivery\n";

    // Test 4: Reject duplicate interface connection.
    assert(!simulator.connectInterfaces(r1, "eth1", r2, "eth2"));

    assert(simulator.getLinkCount() == 1);

    std::cout << "[PASS] Duplicate connection rejected\n";

    // Test 5: Reject transmission using the wrong source MAC.
    EthernetFrame invalidFrame(
        interface2->macAddress,
        interface1->macAddress,
        EtherType::IPv4,
        {0x45, 0x00, 0x00, 0x14}
    );

    assert(!simulator.transmitFrame(r1, "eth1", invalidFrame));

    std::cout << "[PASS] Invalid source MAC rejected\n";

    // Test 6: Reject invalid interface.
    assert(!simulator.transmitFrame(r1, "eth99", broadcastFrame));

    std::cout << "[PASS] Invalid interface rejected\n";

    std::cout << "\nAll Ethernet link tests passed!\n";

    return 0;
}