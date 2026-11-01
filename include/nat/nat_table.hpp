#ifndef NAT_TABLE_HPP
#define NAT_TABLE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <cstddef>

enum class NatProtocol : uint8_t {
    TCP = 6,
    UDP = 17
};

struct NatMapping {
    uint32_t privateIP{};
    uint16_t privatePort{};

    uint32_t publicIP{};
    uint16_t publicPort{};

    uint32_t remoteIP{};
    uint16_t remotePort{};

    NatProtocol protocol{NatProtocol::TCP};
};

struct NatLookupResult {
    bool found{false};
    NatMapping mapping;
};

class NatTable {
private:
    std::unordered_map<std::string, NatMapping> mappings;

    std::string createOutboundKey(
        uint32_t privateIP,
        uint16_t privatePort,
        uint32_t remoteIP,
        uint16_t remotePort,
        NatProtocol protocol
    ) const;

    std::string createInboundKey(
        uint32_t publicIP,
        uint16_t publicPort,
        uint32_t remoteIP,
        uint16_t remotePort,
        NatProtocol protocol
    ) const;

public:
    bool addMapping(const NatMapping& mapping);

    NatLookupResult findOutbound(
        uint32_t privateIP,
        uint16_t privatePort,
        uint32_t remoteIP,
        uint16_t remotePort,
        NatProtocol protocol
    ) const;

    NatLookupResult findInbound(
        uint32_t publicIP,
        uint16_t publicPort,
        uint32_t remoteIP,
        uint16_t remotePort,
        NatProtocol protocol
    ) const;

    bool removeMapping(
        uint32_t privateIP,
        uint16_t privatePort,
        uint32_t remoteIP,
        uint16_t remotePort,
        NatProtocol protocol
    );

    void clear();

    size_t getMappingCount() const;
};

#endif