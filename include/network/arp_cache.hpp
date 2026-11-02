#ifndef ARP_CACHE_HPP
#define ARP_CACHE_HPP

#include <cstdint>
#include <chrono>
#include <unordered_map>
#include <optional>

#include "network/mac_address.hpp"

struct ArpEntry {

    uint32_t ipAddress{};

    MacAddress macAddress;

    std::chrono::steady_clock::time_point expiresAt;
};

class ArpCache {

private:

    std::unordered_map<uint32_t, ArpEntry> entries;

    std::chrono::seconds cacheLifetime;

public:

    explicit ArpCache(
        std::chrono::seconds lifetime =
            std::chrono::seconds(60)
    );

    void addEntry(
        uint32_t ipAddress,
        const MacAddress& macAddress
    );

    void addEntry(
        uint32_t ipAddress,
        const MacAddress& macAddress,
        std::chrono::steady_clock::time_point now
    );

    std::optional<MacAddress> lookup(
        uint32_t ipAddress
    );

    std::optional<MacAddress> lookup(
        uint32_t ipAddress,
        std::chrono::steady_clock::time_point now
    );

    bool removeEntry(uint32_t ipAddress);

    void expireEntries();

    void expireEntries(
        std::chrono::steady_clock::time_point now
    );

    void clear();

    size_t getEntryCount() const;

    // Read-only access to cached ARP entries.
    const std::unordered_map<uint32_t, ArpEntry>& getEntries() const {
        return entries;
    }
};

#endif