#include "nat/nat_engine.hpp"

NatEngine::NatEngine(uint32_t publicIPAddress)
    : publicIP(publicIPAddress),
      nextAvailablePort(MIN_PORT) {
}

bool NatEngine::isSupportedProtocol(uint8_t protocol) const {
    return protocol == 6 || protocol == 17;
}

NatProtocol NatEngine::convertProtocol(uint8_t protocol) const {
    if (protocol == 6) {
        return NatProtocol::TCP;
    }

    return NatProtocol::UDP;
}

bool NatEngine::allocatePort(
    const Packet& packet,
    uint16_t& allocatedPort
) {
    const NatProtocol protocol = convertProtocol(packet.protocol);

    const uint32_t availablePortCount =
        static_cast<uint32_t>(MAX_PORT) - MIN_PORT + 1;

    for (uint32_t attempt = 0;
         attempt < availablePortCount;
         ++attempt) {

        const uint16_t candidatePort = nextAvailablePort;

        // Advance the allocator and wrap around when necessary.
        if (nextAvailablePort >= MAX_PORT) {
            nextAvailablePort = MIN_PORT;
        } else {
            ++nextAvailablePort;
        }

        // Check whether this public endpoint is already occupied
        // for the same remote endpoint and protocol.
        NatLookupResult existing = natTable.findInbound(
            publicIP,
            candidatePort,
            packet.destinationIP,
            packet.destinationPort,
            protocol
        );

        if (!existing.found) {
            allocatedPort = candidatePort;
            return true;
        }
    }

    return false;
}

bool NatEngine::translateOutbound(Packet& packet) {

    if (!isSupportedProtocol(packet.protocol)) {
        return false;
    }

    if (packet.sourcePort == 0 ||
        packet.destinationPort == 0) {
        return false;
    }

    const NatProtocol protocol = convertProtocol(packet.protocol);

    // Reuse an existing mapping when this connection already exists.
    NatLookupResult existing = natTable.findOutbound(
        packet.sourceIP,
        packet.sourcePort,
        packet.destinationIP,
        packet.destinationPort,
        protocol
    );

    if (existing.found) {
        packet.sourceIP = existing.mapping.publicIP;
        packet.sourcePort = existing.mapping.publicPort;

        return true;
    }

    // Allocate a new public source port.
    uint16_t allocatedPort{};

    if (!allocatePort(packet, allocatedPort)) {
        return false;
    }

    NatMapping mapping{
        packet.sourceIP,
        packet.sourcePort,
        publicIP,
        allocatedPort,
        packet.destinationIP,
        packet.destinationPort,
        protocol
    };

    // Store the translation before modifying the packet.
    if (!natTable.addMapping(mapping)) {
        return false;
    }

    packet.sourceIP = publicIP;
    packet.sourcePort = allocatedPort;

    return true;
}

bool NatEngine::translateInbound(Packet& packet) {

    if (!isSupportedProtocol(packet.protocol)) {
        return false;
    }

    if (packet.sourcePort == 0 ||
        packet.destinationPort == 0) {
        return false;
    }

    const NatProtocol protocol = convertProtocol(packet.protocol);

    // Find the original private endpoint using the public
    // destination and the remote source endpoint.
    NatLookupResult result = natTable.findInbound(
        packet.destinationIP,
        packet.destinationPort,
        packet.sourceIP,
        packet.sourcePort,
        protocol
    );

    if (!result.found) {
        return false;
    }

    // Restore the original private destination.
    packet.destinationIP = result.mapping.privateIP;
    packet.destinationPort = result.mapping.privatePort;

    return true;
}

NatTable& NatEngine::getNatTable() {
    return natTable;
}

const NatTable& NatEngine::getNatTable() const {
    return natTable;
}

uint32_t NatEngine::getPublicIP() const {
    return publicIP;
}