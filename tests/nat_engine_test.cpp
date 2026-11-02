#include "nat/nat_engine.hpp"

#include <cassert>
#include <iostream>

constexpr uint32_t PRIVATE_IP = 0xC0A8010A;  // 192.168.1.10
constexpr uint32_t PUBLIC_IP  = 0xCB007101;  // 203.0.113.1
constexpr uint32_t REMOTE_IP  = 0x08080808;  // 8.8.8.8

void testTCPOutboundTranslation() {
    NatEngine engine(PUBLIC_IP);

    Packet packet;
    packet.sourceIP = PRIVATE_IP;
    packet.sourcePort = 5000;
    packet.destinationIP = REMOTE_IP;
    packet.destinationPort = 443;
    packet.protocol = 6;

    bool result = engine.translateOutbound(packet);

    assert(result);
    assert(packet.sourceIP == PUBLIC_IP);
    assert(packet.sourcePort == 40000);
    assert(packet.destinationIP == REMOTE_IP);
    assert(packet.destinationPort == 443);

    std::cout << "[PASS] TCP outbound translation\n";
}

void testUDPOutboundTranslation() {
    NatEngine engine(PUBLIC_IP);

    Packet packet;
    packet.sourceIP = PRIVATE_IP;
    packet.sourcePort = 6000;
    packet.destinationIP = REMOTE_IP;
    packet.destinationPort = 53;
    packet.protocol = 17;

    bool result = engine.translateOutbound(packet);

    assert(result);
    assert(packet.sourceIP == PUBLIC_IP);
    assert(packet.sourcePort == 40000);

    std::cout << "[PASS] UDP outbound translation\n";
}

void testInboundReverseTranslation() {
    NatEngine engine(PUBLIC_IP);

    Packet outbound;
    outbound.sourceIP = PRIVATE_IP;
    outbound.sourcePort = 5000;
    outbound.destinationIP = REMOTE_IP;
    outbound.destinationPort = 443;
    outbound.protocol = 6;

    assert(engine.translateOutbound(outbound));

    Packet response;
    response.sourceIP = REMOTE_IP;
    response.sourcePort = 443;
    response.destinationIP = PUBLIC_IP;
    response.destinationPort = 40000;
    response.protocol = 6;

    bool result = engine.translateInbound(response);

    assert(result);
    assert(response.destinationIP == PRIVATE_IP);
    assert(response.destinationPort == 5000);
    assert(response.sourceIP == REMOTE_IP);
    assert(response.sourcePort == 443);

    std::cout << "[PASS] TCP inbound reverse translation\n";
}

void testUDPInboundReverseTranslation() {
    NatEngine engine(PUBLIC_IP);

    Packet outbound;
    outbound.sourceIP = PRIVATE_IP;
    outbound.sourcePort = 6000;
    outbound.destinationIP = REMOTE_IP;
    outbound.destinationPort = 53;
    outbound.protocol = 17;

    assert(engine.translateOutbound(outbound));

    Packet response;
    response.sourceIP = REMOTE_IP;
    response.sourcePort = 53;
    response.destinationIP = PUBLIC_IP;
    response.destinationPort = 40000;
    response.protocol = 17;

    bool result = engine.translateInbound(response);

    assert(result);
    assert(response.destinationIP == PRIVATE_IP);
    assert(response.destinationPort == 6000);

    std::cout << "[PASS] UDP inbound reverse translation\n";
}

void testExistingMappingReuse() {
    NatEngine engine(PUBLIC_IP);

    Packet first;
    first.sourceIP = PRIVATE_IP;
    first.sourcePort = 5000;
    first.destinationIP = REMOTE_IP;
    first.destinationPort = 443;
    first.protocol = 6;

    assert(engine.translateOutbound(first));

    uint16_t allocatedPort = first.sourcePort;

    Packet second;
    second.sourceIP = PRIVATE_IP;
    second.sourcePort = 5000;
    second.destinationIP = REMOTE_IP;
    second.destinationPort = 443;
    second.protocol = 6;

    assert(engine.translateOutbound(second));

    assert(second.sourceIP == PUBLIC_IP);
    assert(second.sourcePort == allocatedPort);
    assert(engine.getNatTable().getMappingCount() == 1);

    std::cout << "[PASS] Existing mapping reuse\n";
}

void testDifferentConnectionsGetDifferentPorts() {
    NatEngine engine(PUBLIC_IP);

    Packet first;
    first.sourceIP = PRIVATE_IP;
    first.sourcePort = 5000;
    first.destinationIP = REMOTE_IP;
    first.destinationPort = 443;
    first.protocol = 6;

    Packet second;
    second.sourceIP = PRIVATE_IP;
    second.sourcePort = 5001;
    second.destinationIP = REMOTE_IP;
    second.destinationPort = 443;
    second.protocol = 6;

    assert(engine.translateOutbound(first));
    assert(engine.translateOutbound(second));

    assert(first.sourcePort == 40000);
    assert(second.sourcePort == 40001);
    assert(engine.getNatTable().getMappingCount() == 2);

    std::cout << "[PASS] Different connections get different ports\n";
}

void testUnknownInboundPacketRejected() {
    NatEngine engine(PUBLIC_IP);

    Packet packet;
    packet.sourceIP = REMOTE_IP;
    packet.sourcePort = 443;
    packet.destinationIP = PUBLIC_IP;
    packet.destinationPort = 45000;
    packet.protocol = 6;

    assert(!engine.translateInbound(packet));

    std::cout << "[PASS] Unknown inbound packet rejection\n";
}

void testUnsupportedProtocolRejected() {
    NatEngine engine(PUBLIC_IP);

    Packet packet;
    packet.sourceIP = PRIVATE_IP;
    packet.sourcePort = 5000;
    packet.destinationIP = REMOTE_IP;
    packet.destinationPort = 443;
    packet.protocol = 1;

    assert(!engine.translateOutbound(packet));

    std::cout << "[PASS] Unsupported protocol rejection\n";
}

void testInvalidPortsRejected() {
    NatEngine engine(PUBLIC_IP);

    Packet packet;
    packet.sourceIP = PRIVATE_IP;
    packet.sourcePort = 0;
    packet.destinationIP = REMOTE_IP;
    packet.destinationPort = 443;
    packet.protocol = 6;

    assert(!engine.translateOutbound(packet));

    std::cout << "[PASS] Invalid port rejection\n";
}

void testPublicIPAddress() {
    NatEngine engine(PUBLIC_IP);

    assert(engine.getPublicIP() == PUBLIC_IP);

    std::cout << "[PASS] Public IP configuration\n";
}

int main() {
    testTCPOutboundTranslation();
    testUDPOutboundTranslation();
    testInboundReverseTranslation();
    testUDPInboundReverseTranslation();
    testExistingMappingReuse();
    testDifferentConnectionsGetDifferentPorts();
    testUnknownInboundPacketRejected();
    testUnsupportedProtocolRejected();
    testInvalidPortsRejected();
    testPublicIPAddress();

    std::cout << "\nAll NAT engine tests passed!\n";

    return 0;
}