#ifndef ARP_ENGINE_HPP
#define ARP_ENGINE_HPP

#include <cstdint>
#include <optional>

#include "network/mac_address.hpp"
#include "network/arp_cache.hpp"

enum class ArpOperation {
    Request = 1,
    Reply = 2
};

struct ArpMessage {
    ArpOperation operation{ArpOperation::Request};

    uint32_t senderIP{};
    MacAddress senderMAC;

    uint32_t targetIP{};
    MacAddress targetMAC;
};

class ArpEngine {

private:
    uint32_t localIP;
    MacAddress localMAC;

    ArpCache arpCache;

public:
    ArpEngine(
        uint32_t ipAddress,
        const MacAddress& macAddress
    );

    ArpMessage createRequest(uint32_t targetIP) const;

    std::optional<ArpMessage> processRequest(
        const ArpMessage& request
    );

    bool processReply(const ArpMessage& reply);

    std::optional<MacAddress> resolve(uint32_t ipAddress);

    ArpCache& getArpCache();

    const ArpCache& getArpCache() const;

    uint32_t getLocalIP() const;

    const MacAddress& getLocalMAC() const;
};

#endif