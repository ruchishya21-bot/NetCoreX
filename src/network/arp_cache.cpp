#include "network/arp_cache.hpp"

#include <cstddef>

ArpCache::ArpCache(
    std::chrono::seconds lifetime
)
    : cacheLifetime(lifetime) {
}

void ArpCache::addEntry(
    uint32_t ipAddress,
    const MacAddress& macAddress
) {

    addEntry(
        ipAddress,
        macAddress,
        std::chrono::steady_clock::now()
    );
}

void ArpCache::addEntry(
    uint32_t ipAddress,
    const MacAddress& macAddress,
    std::chrono::steady_clock::time_point now
) {

    ArpEntry entry;

    entry.ipAddress = ipAddress;
    entry.macAddress = macAddress;
    entry.expiresAt = now + cacheLifetime;

    entries[ipAddress] = entry;
}

std::optional<MacAddress> ArpCache::lookup(
    uint32_t ipAddress
) {

    return lookup(
        ipAddress,
        std::chrono::steady_clock::now()
    );
}

std::optional<MacAddress> ArpCache::lookup(
    uint32_t ipAddress,
    std::chrono::steady_clock::time_point now
) {

    auto iterator = entries.find(ipAddress);

    if (iterator == entries.end()) {
        return std::nullopt;
    }

    if (now >= iterator->second.expiresAt) {

        entries.erase(iterator);

        return std::nullopt;
    }

    return iterator->second.macAddress;
}

bool ArpCache::removeEntry(uint32_t ipAddress) {

    return entries.erase(ipAddress) > 0;
}

void ArpCache::expireEntries() {

    expireEntries(
        std::chrono::steady_clock::now()
    );
}

void ArpCache::expireEntries(
    std::chrono::steady_clock::time_point now
) {

    for (auto iterator = entries.begin();
         iterator != entries.end();) {

        if (now >= iterator->second.expiresAt) {

            iterator = entries.erase(iterator);

        }
        else {

            ++iterator;
        }
    }
}

void ArpCache::clear() {

    entries.clear();
}

size_t ArpCache::getEntryCount() const {

    return entries.size();
}