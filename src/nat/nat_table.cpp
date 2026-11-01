#include "nat/nat_table.hpp"

#include <cstddef>
#include <string>

std::string NatTable::createOutboundKey(
    uint32_t privateIP,
    uint16_t privatePort,
    uint32_t remoteIP,
    uint16_t remotePort,
    NatProtocol protocol
) const {
    return std::to_string(privateIP) + ":" +
           std::to_string(privatePort) + ":" +
           std::to_string(remoteIP) + ":" +
           std::to_string(remotePort) + ":" +
           std::to_string(static_cast<uint8_t>(protocol));
}

std::string NatTable::createInboundKey(
    uint32_t publicIP,
    uint16_t publicPort,
    uint32_t remoteIP,
    uint16_t remotePort,
    NatProtocol protocol
) const {
    return std::to_string(publicIP) + ":" +
           std::to_string(publicPort) + ":" +
           std::to_string(remoteIP) + ":" +
           std::to_string(remotePort) + ":" +
           std::to_string(static_cast<uint8_t>(protocol));
}

bool NatTable::addMapping(const NatMapping& mapping) {

    if (mapping.protocol != NatProtocol::TCP &&
        mapping.protocol != NatProtocol::UDP) {
        return false;
    }

    if (mapping.privatePort == 0 ||
        mapping.publicPort == 0 ||
        mapping.remotePort == 0) {
        return false;
    }

    const std::string outboundKey = createOutboundKey(
        mapping.privateIP,
        mapping.privatePort,
        mapping.remoteIP,
        mapping.remotePort,
        mapping.protocol
    );

    const std::string inboundKey = createInboundKey(
        mapping.publicIP,
        mapping.publicPort,
        mapping.remoteIP,
        mapping.remotePort,
        mapping.protocol
    );

    if (mappings.find(outboundKey) != mappings.end()) {
        return false;
    }

    for (const auto& entry : mappings) {
        const NatMapping& existing = entry.second;

        const std::string existingInboundKey = createInboundKey(
            existing.publicIP,
            existing.publicPort,
            existing.remoteIP,
            existing.remotePort,
            existing.protocol
        );

        if (existingInboundKey == inboundKey) {
            return false;
        }
    }

    mappings.emplace(outboundKey, mapping);

    return true;
}

NatLookupResult NatTable::findOutbound(
    uint32_t privateIP,
    uint16_t privatePort,
    uint32_t remoteIP,
    uint16_t remotePort,
    NatProtocol protocol
) const {

    const std::string key = createOutboundKey(
        privateIP,
        privatePort,
        remoteIP,
        remotePort,
        protocol
    );

    auto it = mappings.find(key);

    if (it == mappings.end()) {
        return {};
    }

    return {true, it->second};
}

NatLookupResult NatTable::findInbound(
    uint32_t publicIP,
    uint16_t publicPort,
    uint32_t remoteIP,
    uint16_t remotePort,
    NatProtocol protocol
) const {

    const std::string key = createInboundKey(
        publicIP,
        publicPort,
        remoteIP,
        remotePort,
        protocol
    );

    for (const auto& entry : mappings) {
        const NatMapping& mapping = entry.second;

        const std::string existingKey = createInboundKey(
            mapping.publicIP,
            mapping.publicPort,
            mapping.remoteIP,
            mapping.remotePort,
            mapping.protocol
        );

        if (existingKey == key) {
            return {true, mapping};
        }
    }

    return {};
}

bool NatTable::removeMapping(
    uint32_t privateIP,
    uint16_t privatePort,
    uint32_t remoteIP,
    uint16_t remotePort,
    NatProtocol protocol
) {

    const std::string key = createOutboundKey(
        privateIP,
        privatePort,
        remoteIP,
        remotePort,
        protocol
    );

    return mappings.erase(key) > 0;
}

void NatTable::clear() {
    mappings.clear();
}

size_t NatTable::getMappingCount() const {
    return mappings.size();
}