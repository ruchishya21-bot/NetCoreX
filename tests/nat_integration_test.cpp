#include <cassert>
#include <iostream>

#include "network/network_simulator.hpp"

void testOutboundAndInboundNat() {

    Router router("NAT-Router");

    const uint32_t privateIP = 0xC0A8010A; // 192.168.1.10
    const uint32_t remoteIP  = 0xC6336414; // 198.51.100.20
    const uint32_t publicIP  = 0xCB007101; // 203.0.113.1

    router.addInterface("ethLAN", 0xC0A80101);
    router.addInterface("ethWAN", publicIP);

    router.addRoute(
        0xC0A80100,
        24,
        0,
        "ethLAN"
    );

    router.addRoute(
        0xC6336400,
        24,
        0,
        "ethWAN"
    );

    router.configureNat(publicIP);

    NetworkSimulator simulator;
    simulator.addRouter(router);

    // ==========================================
    // OUTBOUND PACKET
    // ==========================================

    Packet outbound;

    outbound.packetId = 1;
    outbound.sourceIP = privateIP;
    outbound.destinationIP = remoteIP;
    outbound.sourcePort = 5000;
    outbound.destinationPort = 443;
    outbound.protocol = 6;
    outbound.tcpFlags = 0x02;

    ForwardingResult outboundResult =
        simulator.simulate(outbound, router);

    assert(outboundResult == ForwardingResult::Delivered);

    assert(outbound.sourceIP == publicIP);
    assert(outbound.sourcePort >= 40000);
    assert(outbound.sourcePort <= 60999);

    const uint16_t translatedPort = outbound.sourcePort;

    std::cout << "Outbound NAT test passed.\n";

    // ==========================================
    // INBOUND RESPONSE
    // ==========================================

    Packet inbound;

    inbound.packetId = 2;
    inbound.sourceIP = remoteIP;
    inbound.destinationIP = publicIP;
    inbound.sourcePort = 443;
    inbound.destinationPort = translatedPort;
    inbound.protocol = 6;
    inbound.tcpFlags = 0x12;

    ForwardingResult inboundResult =
        simulator.simulate(inbound, router);

    assert(inboundResult == ForwardingResult::Delivered);

    assert(inbound.destinationIP == privateIP);
    assert(inbound.destinationPort == 5000);

    std::cout << "Inbound NAT test passed.\n";
}

int main() {

    testOutboundAndInboundNat();

    std::cout << "\nAll NAT integration tests passed.\n";

    return 0;
}