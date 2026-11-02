#ifndef ARP_NETWORK_HPP
#define ARP_NETWORK_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "network/arp_engine.hpp"
#include "network/lan_segment.hpp"

class ArpNetwork {

private:
    struct Device {
        std::unique_ptr<ArpEngine> arpEngine;
    };

    LanSegment lan;
    std::unordered_map<uint32_t, Device> devices;

    static std::vector<uint8_t> serialize(
        const ArpMessage& message
    );

    static bool deserialize(
        const std::vector<uint8_t>& payload,
        ArpMessage& message
    );

    void handleFrame(
        uint32_t deviceIP,
        const EthernetFrame& frame
    );

public:
    bool addDevice(
        uint32_t ip,
        const MacAddress& mac
    );

    bool removeDevice(uint32_t ip);

    bool requestResolution(
        uint32_t senderIP,
        uint32_t targetIP
    );

    std::optional<MacAddress> resolve(
        uint32_t deviceIP,
        uint32_t targetIP
    );

    std::size_t getDeviceCount() const;

    const LanSegment& getLan() const;
};

#endif