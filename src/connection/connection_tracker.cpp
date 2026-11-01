#include "connection/connection_tracker.hpp"

#include <functional>
#include <utility>

// TCP flag constants.
namespace {

constexpr uint8_t TCP_FIN = 0x01;
constexpr uint8_t TCP_SYN = 0x02;
constexpr uint8_t TCP_RST = 0x04;
constexpr uint8_t TCP_ACK = 0x10;

ConnectionUpdateResult makeResult(
    ConnectionStatus status,
    ConnectionState state,
    const std::string& message
) {
    return {
        status,
        state,
        message
    };
}

}

// Compare connection keys.
bool ConnectionKey::operator==(
    const ConnectionKey& other
) const {

    return sourceIP == other.sourceIP &&
           destinationIP == other.destinationIP &&
           sourcePort == other.sourcePort &&
           destinationPort == other.destinationPort;
}

// Generate a hash for a connection key.
std::size_t ConnectionKeyHash::operator()(
    const ConnectionKey& key
) const {

    std::size_t seed = 0;

    auto combine = [&seed](std::size_t value) {

        seed ^= value +
                0x9e3779b9 +
                (seed << 6) +
                (seed >> 2);
    };

    combine(std::hash<uint32_t>{}(key.sourceIP));
    combine(std::hash<uint32_t>{}(key.destinationIP));
    combine(std::hash<uint16_t>{}(key.sourcePort));
    combine(std::hash<uint16_t>{}(key.destinationPort));

    return seed;
}


// Process a TCP packet.
ConnectionUpdateResult ConnectionTracker::processPacket(
    const Packet& packet
) {

    if (packet.protocol != 6) {

        return makeResult(
            ConnectionStatus::UnsupportedProtocol,
            ConnectionState::CLOSED,
            "Connection tracking supports TCP packets only."
        );
    }

    const bool syn = (packet.tcpFlags & TCP_SYN) != 0;
    const bool ack = (packet.tcpFlags & TCP_ACK) != 0;
    const bool fin = (packet.tcpFlags & TCP_FIN) != 0;
    const bool rst = (packet.tcpFlags & TCP_RST) != 0;

    ConnectionKey forwardKey{
        packet.sourceIP,
        packet.destinationIP,
        packet.sourcePort,
        packet.destinationPort
    };

    ConnectionKey reverseKey{
        packet.destinationIP,
        packet.sourceIP,
        packet.destinationPort,
        packet.sourcePort
    };

    auto it = connections.find(forwardKey);

    bool fromInitiator = true;

    if (it == connections.end()) {

        it = connections.find(reverseKey);

        fromInitiator = false;
    }

    // Handle a new connection.
    if (it == connections.end()) {

        // Only an initial SYN can create a connection.
        if (syn && !ack && !fin && !rst) {

            Connection connection;

            connection.key = forwardKey;
            connection.state = ConnectionState::SYN_SENT;
            connection.initiatorFin = false;
            connection.responderFin = false;
            connection.lastActivity =
                std::chrono::steady_clock::now();

            connections.emplace(
                forwardKey,
                connection
            );

            return makeResult(
                ConnectionStatus::NewConnection,
                ConnectionState::SYN_SENT,
                "New TCP connection detected. SYN received."
            );
        }

        return makeResult(
            ConnectionStatus::InvalidTransition,
            ConnectionState::CLOSED,
            "Packet does not belong to a tracked connection."
        );
    }

    Connection& connection = it->second;

    // Update activity timestamp.
    connection.lastActivity =
        std::chrono::steady_clock::now();

    // Closed connections cannot accept further packets.
    if (connection.state == ConnectionState::CLOSED) {

        return makeResult(
            ConnectionStatus::InvalidTransition,
            connection.state,
            "Connection is already closed."
        );
    }

    // RST immediately closes an existing connection.
    if (rst) {

        connection.state = ConnectionState::CLOSED;

        return makeResult(
            ConnectionStatus::Closed,
            connection.state,
            "TCP reset received. Connection closed."
        );
    }

    // Connection establishment.
    if (connection.state == ConnectionState::SYN_SENT) {

        // The responder must send SYN + ACK.
        if (!fromInitiator && syn && ack && !fin) {

            connection.state =
                ConnectionState::SYN_ACK_RECEIVED;

            return makeResult(
                ConnectionStatus::Updated,
                connection.state,
                "SYN-ACK received from responder."
            );
        }

        // Allow retransmission of the original SYN.
        if (fromInitiator && syn && !ack && !fin) {

            return makeResult(
                ConnectionStatus::Updated,
                connection.state,
                "Original SYN retransmission received."
            );
        }

        return makeResult(
            ConnectionStatus::InvalidTransition,
            connection.state,
            "Invalid packet during TCP handshake."
        );
    }

    if (connection.state == ConnectionState::SYN_ACK_RECEIVED) {

        // The original initiator completes the handshake.
        if (fromInitiator && ack && !syn && !fin) {

            connection.state =
                ConnectionState::ESTABLISHED;

            return makeResult(
                ConnectionStatus::Established,
                connection.state,
                "TCP three-way handshake completed."
            );
        }

        return makeResult(
            ConnectionStatus::InvalidTransition,
            connection.state,
            "Invalid packet during final TCP handshake."
        );
    }

    // Established connection.
    if (connection.state == ConnectionState::ESTABLISHED) {

        if (syn) {

            return makeResult(
                ConnectionStatus::InvalidTransition,
                connection.state,
                "Unexpected SYN on established connection."
            );
        }

        if (fin) {

            if (fromInitiator) {
                connection.initiatorFin = true;
            } else {
                connection.responderFin = true;
            }

            if (connection.initiatorFin &&
                connection.responderFin) {

                connection.state = ConnectionState::CLOSED;

                return makeResult(
                    ConnectionStatus::Closed,
                    connection.state,
                    "FIN received from both endpoints."
                );
            }

            connection.state = ConnectionState::FIN_WAIT;

            return makeResult(
                ConnectionStatus::Updated,
                connection.state,
                "TCP connection termination started."
            );
        }

        return makeResult(
            ConnectionStatus::Updated,
            connection.state,
            "TCP connection remains established."
        );
    }

    // Connection termination.
    if (connection.state == ConnectionState::FIN_WAIT) {

        if (syn) {

            return makeResult(
                ConnectionStatus::InvalidTransition,
                connection.state,
                "Unexpected SYN during connection termination."
            );
        }

        if (fin) {

            if (fromInitiator) {
                connection.initiatorFin = true;
            } else {
                connection.responderFin = true;
            }

            if (connection.initiatorFin &&
                connection.responderFin) {

                connection.state = ConnectionState::CLOSED;

                return makeResult(
                    ConnectionStatus::Closed,
                    connection.state,
                    "Both endpoints have sent FIN."
                );
            }
        }

        return makeResult(
            ConnectionStatus::Updated,
            connection.state,
            "Connection is waiting for termination."
        );
    }

    return makeResult(
        ConnectionStatus::InvalidTransition,
        connection.state,
        "Unsupported TCP state transition."
    );
}


// Retrieve the current state of a connection.
bool ConnectionTracker::getConnectionState(
    const Packet& packet,
    ConnectionState& state
) const {

    ConnectionKey forwardKey{
        packet.sourceIP,
        packet.destinationIP,
        packet.sourcePort,
        packet.destinationPort
    };

    ConnectionKey reverseKey{
        packet.destinationIP,
        packet.sourceIP,
        packet.destinationPort,
        packet.sourcePort
    };

    auto it = connections.find(forwardKey);

    if (it == connections.end()) {
        it = connections.find(reverseKey);
    }

    if (it == connections.end()) {
        return false;
    }

    state = it->second.state;

    return true;
}


// Return the number of tracked connections.
std::size_t ConnectionTracker::getConnectionCount() const {

    return connections.size();
}


// Remove idle connections.
std::size_t ConnectionTracker::expireIdleConnections(
    std::chrono::seconds timeout
) {

    const auto now = std::chrono::steady_clock::now();

    std::size_t removed = 0;

    for (auto it = connections.begin();
         it != connections.end();) {

        if (now - it->second.lastActivity >= timeout) {

            it = connections.erase(it);
            removed++;

        } else {

            ++it;
        }
    }

    return removed;
}


// Remove closed connections.
std::size_t ConnectionTracker::removeClosedConnections() {

    std::size_t removed = 0;

    for (auto it = connections.begin();
         it != connections.end();) {

        if (it->second.state == ConnectionState::CLOSED) {

            it = connections.erase(it);
            removed++;

        } else {

            ++it;
        }
    }

    return removed;
}


// Clear the entire connection table.
void ConnectionTracker::clear() {

    connections.clear();
}