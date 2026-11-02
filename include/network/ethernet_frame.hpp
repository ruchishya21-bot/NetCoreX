#ifndef ETHERNET_FRAME_HPP
#define ETHERNET_FRAME_HPP

#include <cstdint>
#include <vector>

#include "network/mac_address.hpp"

enum class EtherType : uint16_t {
    IPv4 = 0x0800,
    ARP  = 0x0806
};

class EthernetFrame {

private:
    MacAddress destinationMAC;
    MacAddress sourceMAC;

    EtherType etherType;

    std::vector<uint8_t> payload;

public:
    static constexpr std::size_t MAX_PAYLOAD_SIZE = 1500;

    EthernetFrame(
        const MacAddress& source,
        const MacAddress& destination,
        EtherType type,
        const std::vector<uint8_t>& data
    );

    const MacAddress& getSourceMAC() const;

    const MacAddress& getDestinationMAC() const;

    EtherType getEtherType() const;

    const std::vector<uint8_t>& getPayload() const;

    bool isBroadcast() const;

    bool isValid() const;

    static MacAddress broadcastMAC();
};

#endif