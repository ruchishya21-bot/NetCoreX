#ifndef NAT_ENGINE_HPP
#define NAT_ENGINE_HPP

#include <cstdint>

#include "packet/packet.hpp"
#include "nat/nat_table.hpp"

class NatEngine {
private:
    NatTable natTable;

    uint32_t publicIP;

    uint16_t nextAvailablePort;

    static constexpr uint16_t MIN_PORT = 40000;
    static constexpr uint16_t MAX_PORT = 60999;

    bool isSupportedProtocol(uint8_t protocol) const;

    NatProtocol convertProtocol(uint8_t protocol) const;

    bool allocatePort(
        const Packet& packet,
        uint16_t& allocatedPort
    );

public:
    explicit NatEngine(uint32_t publicIPAddress);

    bool translateOutbound(Packet& packet);

    bool translateInbound(Packet& packet);

    NatTable& getNatTable();

    const NatTable& getNatTable() const;

    uint32_t getPublicIP() const;
};

#endif