#ifndef CONNECTION_TRACKER_HPP
#define CONNECTION_TRACKER_HPP

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <string>
#include <unordered_map>

#include "packet/packet.hpp"

// TCP connection states.
enum class ConnectionState {
    SYN_SENT,
    SYN_ACK_RECEIVED,
    ESTABLISHED,
    FIN_WAIT,
    CLOSED
};

// Result of processing a packet.
enum class ConnectionStatus {
    NewConnection,
    Updated,
    Established,
    Closed,
    InvalidTransition,
    UnsupportedProtocol
};

// Connection identification.
// The key preserves the original initiator direction.
struct ConnectionKey {

    uint32_t sourceIP;
    uint32_t destinationIP;

    uint16_t sourcePort;
    uint16_t destinationPort;

    bool operator==(const ConnectionKey& other) const;
};

// Hash function for connection keys.
struct ConnectionKeyHash {

    std::size_t operator()(
        const ConnectionKey& key
    ) const;
};

// Stored connection information.
struct Connection {

    ConnectionKey key;

    ConnectionState state;

    bool initiatorFin;
    bool responderFin;

    std::chrono::steady_clock::time_point lastActivity;
};

// Result returned by the connection tracker.
struct ConnectionUpdateResult {

    ConnectionStatus status;

    ConnectionState state;

    std::string message;
};

class ConnectionTracker {

private:

    std::unordered_map<
        ConnectionKey,
        Connection,
        ConnectionKeyHash
    > connections;

public:

    // Process an incoming TCP packet.
    ConnectionUpdateResult processPacket(
        const Packet& packet
    );

    // Find the current state of a connection.
    bool getConnectionState(
        const Packet& packet,
        ConnectionState& state
    ) const;

    // Return the number of tracked connections.
    std::size_t getConnectionCount() const;

    // Remove connections that have been idle too long.
    std::size_t expireIdleConnections(
        std::chrono::seconds timeout
    );

    // Remove all connections in CLOSED state.
    std::size_t removeClosedConnections();

    // Remove every tracked connection.
    void clear();
};

#endif