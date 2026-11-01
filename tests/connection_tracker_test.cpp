#include <iostream>
#include <cassert>

#include "connection/connection_tracker.hpp"

Packet createTcpPacket(
    uint32_t sourceIP,
    uint32_t destinationIP,
    uint16_t sourcePort,
    uint16_t destinationPort,
    uint8_t flags
) {

    Packet packet;

    packet.sourceIP = sourceIP;
    packet.destinationIP = destinationIP;

    packet.sourcePort = sourcePort;
    packet.destinationPort = destinationPort;

    packet.protocol = 6;
    packet.tcpFlags = flags;

    return packet;
}


// TEST 1: New SYN connection.
void testNewConnection() {

    ConnectionTracker tracker;

    Packet syn = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x02
    );

    auto result = tracker.processPacket(syn);

    assert(result.status == ConnectionStatus::NewConnection);
    assert(result.state == ConnectionState::SYN_SENT);

    assert(tracker.getConnectionCount() == 1);

    std::cout << "[PASS] New TCP connection detection\n";
}


// TEST 2: Complete three-way handshake.
void testTcpHandshake() {

    ConnectionTracker tracker;

    Packet syn = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x02
    );

    Packet synAck = createTcpPacket(
        0x0A000002,
        0x0A000001,
        80,
        5000,
        0x12
    );

    Packet ack = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x10
    );

    assert(
        tracker.processPacket(syn).state ==
        ConnectionState::SYN_SENT
    );

    assert(
        tracker.processPacket(synAck).state ==
        ConnectionState::SYN_ACK_RECEIVED
    );

    auto result = tracker.processPacket(ack);

    assert(result.status == ConnectionStatus::Established);

    assert(result.state == ConnectionState::ESTABLISHED);

    std::cout << "[PASS] TCP three-way handshake\n";
}


// TEST 3: Reverse-direction connection lookup.
void testReverseDirectionLookup() {

    ConnectionTracker tracker;

    Packet syn = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x02
    );

    Packet reverse = createTcpPacket(
        0x0A000002,
        0x0A000001,
        80,
        5000,
        0x12
    );

    tracker.processPacket(syn);

    ConnectionState state;

    assert(tracker.getConnectionState(reverse, state));

    assert(state == ConnectionState::SYN_SENT);

    std::cout << "[PASS] Bidirectional connection lookup\n";
}


// TEST 4: Invalid TCP state transition.
void testInvalidTransition() {

    ConnectionTracker tracker;

    Packet ack = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x10
    );

    auto result = tracker.processPacket(ack);

    assert(result.status == ConnectionStatus::InvalidTransition);

    assert(tracker.getConnectionCount() == 0);

    std::cout << "[PASS] Invalid TCP transition detection\n";
}


// TEST 5: FIN termination.
void testConnectionTermination() {

    ConnectionTracker tracker;

    Packet syn = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x02
    );

    Packet synAck = createTcpPacket(
        0x0A000002,
        0x0A000001,
        80,
        5000,
        0x12
    );

    Packet ack = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x10
    );

    Packet fin1 = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x11
    );

    Packet fin2 = createTcpPacket(
        0x0A000002,
        0x0A000001,
        80,
        5000,
        0x11
    );

    tracker.processPacket(syn);
    tracker.processPacket(synAck);
    tracker.processPacket(ack);

    auto firstFin = tracker.processPacket(fin1);

    assert(firstFin.state == ConnectionState::FIN_WAIT);

    auto secondFin = tracker.processPacket(fin2);

    assert(secondFin.status == ConnectionStatus::Closed);

    assert(secondFin.state == ConnectionState::CLOSED);

    std::cout << "[PASS] TCP FIN termination\n";
}


// TEST 6: RST termination.
void testResetConnection() {

    ConnectionTracker tracker;

    Packet syn = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x02
    );

    Packet rst = createTcpPacket(
        0x0A000002,
        0x0A000001,
        80,
        5000,
        0x04
    );

    tracker.processPacket(syn);

    auto result = tracker.processPacket(rst);

    assert(result.status == ConnectionStatus::Closed);

    assert(result.state == ConnectionState::CLOSED);

    std::cout << "[PASS] TCP RST termination\n";
}


// TEST 7: Remove closed connections.
void testClosedConnectionCleanup() {

    ConnectionTracker tracker;

    Packet syn = createTcpPacket(
        0x0A000001,
        0x0A000002,
        5000,
        80,
        0x02
    );

    Packet rst = createTcpPacket(
        0x0A000002,
        0x0A000001,
        80,
        5000,
        0x04
    );

    tracker.processPacket(syn);
    tracker.processPacket(rst);

    assert(tracker.getConnectionCount() == 1);

    std::size_t removed = tracker.removeClosedConnections();

    assert(removed == 1);

    assert(tracker.getConnectionCount() == 0);

    std::cout << "[PASS] Closed connection cleanup\n";
}


// TEST 8: Unsupported protocol.
void testUnsupportedProtocol() {

    ConnectionTracker tracker;

    Packet udp;

    udp.protocol = 17;

    auto result = tracker.processPacket(udp);

    assert(
        result.status == ConnectionStatus::UnsupportedProtocol
    );

    std::cout << "[PASS] Unsupported protocol detection\n";
}


int main() {

    std::cout << "\n========== CONNECTION TRACKER TESTS ==========\n";

    testNewConnection();
    testTcpHandshake();
    testReverseDirectionLookup();
    testInvalidTransition();
    testConnectionTermination();
    testResetConnection();
    testClosedConnectionCleanup();
    testUnsupportedProtocol();

    std::cout << "\nALL CONNECTION TRACKER TESTS PASSED\n";

    return 0;
}