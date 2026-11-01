#include <iostream>
#include <cassert>

#include "network/network_simulator.hpp"
#include "routing/control_plane.hpp"

struct TestNetwork {

    Router r1{"Router-1"};
    Router r2{"Router-2"};
    Router r3{"Router-3"};
    Router r4{"Router-4"};

    ControlPlane controlPlane;
    NetworkSimulator simulator;

    TestNetwork() {

        // Router-1 interfaces
        r1.addInterface("eth12", 0x0A000001);
        r1.addInterface("eth13", 0x0A000002);

        r1.addInterface("ethLAN", 0xC0A80101);

r1.addRoute(
    0xC0A80100,
    24,
    0,
    "ethLAN"
);

        // Router-2 interfaces
        r2.addInterface("eth21", 0x0A000011);
        r2.addInterface("eth24", 0x0A000012);

        // Router-3 interfaces
        r3.addInterface("eth31", 0x0A000021);
        r3.addInterface("eth34", 0x0A000022);

        // Router-4 interfaces
        r4.addInterface("eth42", 0x0A000031);
        r4.addInterface("eth43", 0x0A000032);
        r4.addInterface("ethLAN", 0xC0A80201);

        // Register routers
        controlPlane.registerRouter(r1);
        controlPlane.registerRouter(r2);
        controlPlane.registerRouter(r3);
        controlPlane.registerRouter(r4);

        // Primary path: Router-1 -> Router-2 -> Router-4
        assert(controlPlane.addLink(
            "Router-1", "eth12",
            "Router-2", "eth21", 2
        ));

        assert(controlPlane.addLink(
            "Router-2", "eth24",
            "Router-4", "eth42", 2
        ));

        // Alternate path: Router-1 -> Router-3 -> Router-4
        assert(controlPlane.addLink(
            "Router-1", "eth13",
            "Router-3", "eth31", 5
        ));

        assert(controlPlane.addLink(
            "Router-3", "eth34",
            "Router-4", "eth43", 5
        ));

        // Destination network
        assert(controlPlane.advertiseNetwork(
            "Router-4",
            0xC0A80200,
            24,
            "ethLAN"
        ));

        controlPlane.installRoutes();

        // Add routers to simulator
        simulator.addRouter(r1);
        simulator.addRouter(r2);
        simulator.addRouter(r3);
        simulator.addRouter(r4);
    }

    Packet createPacket(uint8_t ttl = 64) {

        Packet packet;

        packet.sourceIP = 0xC0A80101;
        packet.destinationIP = 0xC0A80202;
        packet.ttl = ttl;
        packet.payload = {1, 2, 3, 4};

        return packet;
    }
};


// TEST 1: Normal packet delivery
void testSuccessfulForwarding() {

    TestNetwork network;

    Packet packet = network.createPacket();

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::Delivered);
    assert(packet.ttl == 61);

    std::cout << "PASS: Successful packet forwarding\n";
}


// TEST 2: TTL expiration
void testTtlExpiration() {

    TestNetwork network;

    Packet packet = network.createPacket(2);

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::TtlExpired);

    assert(network.simulator.getLastIcmpError().has_value());

    const auto& error =
        network.simulator.getLastIcmpError().value();

    assert(error.type == IcmpType::TimeExceeded);
    assert(error.code == IcmpCode::TtlExpired);
    assert(error.reportingRouter == "Router-2");
    assert(error.originalDestinationIP == 0xC0A80202);

    std::cout << "PASS: TTL expiration and ICMP error generation\n";
}


// TEST 3: Missing route
void testMissingRoute() {

    TestNetwork network;

    Packet packet = network.createPacket();

    packet.destinationIP = 0x0A0000FF;

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::NoRoute);

    assert(network.simulator.getLastIcmpError().has_value());

    const auto& error =
        network.simulator.getLastIcmpError().value();

    assert(error.type == IcmpType::DestinationUnreachable);
    assert(error.code == IcmpCode::NetworkUnreachable);
    assert(error.reportingRouter == "Router-1");

    std::cout << "PASS: Missing route and ICMP error generation\n";
}

// TEST 4: Primary link failure and alternate path
void testFailoverForwarding() {

    TestNetwork network;

    // Remove primary link
    network.controlPlane.removeLink(
        "Router-2",
        "Router-4"
    );

    // Recalculate and install routes
    network.controlPlane.installRoutes();

    Packet packet = network.createPacket();

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::Delivered);
    assert(packet.ttl == 61);

    std::cout << "PASS: Alternate path forwarding after link failure\n";
}

// TEST 5: ACL denies a packet
void testAclDeniesPacket() {

    TestNetwork network;

    ACLRule denyRule{
        0xC0A80101, 32,  // Source: 192.168.1.1
        0xC0A80202, 32,  // Destination: 192.168.2.2
        0,               // Any protocol
        ACLAction::Deny
    };

    network.r1.getAccessControlList().addRule(denyRule);
    network.r1.setAccessControlEnabled(true);

    Packet packet = network.createPacket();

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::AclDenied);
    assert(packet.ttl == 64);

    std::cout << "PASS: ACL denies prohibited packet\n";
}


// TEST 6: ACL permits a packet
void testAclPermitsPacket() {

    TestNetwork network;

    ACLRule permitRule{
        0xC0A80101, 32,
        0xC0A80202, 32,
        0,
        ACLAction::Permit
    };

    network.r1.getAccessControlList().addRule(permitRule);
    network.r1.setAccessControlEnabled(true);

    Packet packet = network.createPacket();

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::Delivered);
    assert(packet.ttl == 61);

    std::cout << "PASS: ACL permits authorized packet\n";
}


// TEST 7: ACL default-denies unmatched packets
void testAclDefaultDeny() {

    TestNetwork network;

    // Enable ACL without adding any rules.
    network.r1.setAccessControlEnabled(true);

    Packet packet = network.createPacket();

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::AclDenied);
    assert(packet.ttl == 64);

    std::cout << "PASS: ACL default-denies unmatched packet\n";
}


// TEST 8: Stateful TCP handshake and established traffic.
void testStatefulTcpConnection() {

    TestNetwork network;

    network.r1.setAccessControlEnabled(true);
    network.r1.setStatefulFirewallEnabled(true);

    ACLRule permitRule{
        0xC0A80101, 32,
        0xC0A80202, 32,
        6,
        ACLAction::Permit
    };

    network.r1.getAccessControlList().addRule(permitRule);

    // Step 1: Initial SYN.
    Packet syn = network.createPacket();

    syn.protocol = 6;
    syn.sourcePort = 50000;
    syn.destinationPort = 443;
    syn.tcpFlags = 0x02;

    assert(
        network.simulator.simulate(syn, network.r1)
        == ForwardingResult::Delivered
    );

    // Step 2: SYN-ACK from responder.
    Packet synAck = network.createPacket();

    synAck.sourceIP = 0xC0A80202;
    synAck.destinationIP = 0xC0A80101;
    synAck.sourcePort = 443;
    synAck.destinationPort = 50000;
    synAck.protocol = 6;
    synAck.tcpFlags = 0x12;

    assert(
        network.simulator.simulate(synAck, network.r1)
        == ForwardingResult::Delivered
    );

    // Step 3: Final ACK.
    Packet ack = network.createPacket();

    ack.protocol = 6;
    ack.sourcePort = 50000;
    ack.destinationPort = 443;
    ack.tcpFlags = 0x10;

    assert(
        network.simulator.simulate(ack, network.r1)
        == ForwardingResult::Delivered
    );

    // Step 4: Reverse established traffic.
    // The original ACL only permits the forward direction.
    // Stateful tracking should permit this response.

    Packet response = network.createPacket();

    response.sourceIP = 0xC0A80202;
    response.destinationIP = 0xC0A80101;
    response.sourcePort = 443;
    response.destinationPort = 50000;
    response.protocol = 6;
    response.tcpFlags = 0x10;

    assert(
        network.simulator.simulate(response, network.r1)
        == ForwardingResult::Delivered
    );

    assert(
        network.r1.getConnectionTracker().getConnectionCount() == 1
    );

    std::cout
        << "PASS: Stateful TCP handshake and established traffic\n";
}


// TEST 9: Unknown TCP ACK is rejected.
void testStatefulRejectsUnknownAck() {

    TestNetwork network;

    network.r1.setStatefulFirewallEnabled(true);

    Packet packet = network.createPacket();

    packet.protocol = 6;
    packet.sourcePort = 50001;
    packet.destinationPort = 443;
    packet.tcpFlags = 0x10;

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(
        result == ForwardingResult::StatefulFirewallDenied
    );

    assert(
        network.r1.getConnectionTracker().getConnectionCount() == 0
    );

    std::cout
        << "PASS: Stateful firewall rejects unknown TCP ACK\n";
}


// TEST 10: ACL rejects a new TCP connection.
void testStatefulAclDeniesNewConnection() {

    TestNetwork network;

    network.r1.setAccessControlEnabled(true);
    network.r1.setStatefulFirewallEnabled(true);

    // No permit rule is installed.
    // ACL default-deny should reject the initial SYN.

    Packet packet = network.createPacket();

    packet.protocol = 6;
    packet.sourcePort = 50000;
    packet.destinationPort = 443;
    packet.tcpFlags = 0x02;

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::AclDenied);

    // A denied connection must not be recorded.
    assert(
        network.r1.getConnectionTracker().getConnectionCount() == 0
    );

    std::cout
        << "PASS: Stateful firewall applies ACL to new TCP connections\n";
}


// TEST 11: Stateful firewall permits UDP when ACL is disabled.
void testStatefulUdpWithoutAcl() {

    TestNetwork network;

    network.r1.setStatefulFirewallEnabled(true);

    Packet packet = network.createPacket();

    packet.protocol = 17;
    packet.sourcePort = 40000;
    packet.destinationPort = 53;

    ForwardingResult result =
        network.simulator.simulate(packet, network.r1);

    assert(result == ForwardingResult::Delivered);

    std::cout
        << "PASS: Stateful firewall preserves UDP forwarding without ACL\n";
}


int main() {

    std::cout
        << "\n========== NETWORK SIMULATOR TESTS ==========\n";

    testSuccessfulForwarding();
    testTtlExpiration();
    testMissingRoute();
    testFailoverForwarding();

    testAclDeniesPacket();
    testAclPermitsPacket();
    testAclDefaultDeny();

    testStatefulTcpConnection();
    testStatefulRejectsUnknownAck();
    testStatefulAclDeniesNewConnection();
    testStatefulUdpWithoutAcl();

    std::cout
        << "\nALL NETWORK SIMULATOR TESTS PASSED\n";

    return 0;
}