#include <cassert>
#include <chrono>
#include <iostream>

#include "network/mac_address.hpp"
#include "network/arp_cache.hpp"

void testMacAddressParsing() {

    auto mac = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(mac.has_value());

    assert(
        mac->toString() == "aa:bb:cc:dd:ee:ff"
    );

    std::cout << "PASS: MAC address parsing\n";
}

void testInvalidMacAddress() {

    auto mac = MacAddress::parse("AA:BB:CC");

    assert(!mac.has_value());

    auto invalid = MacAddress::parse("GG:BB:CC:DD:EE:FF");

    assert(!invalid.has_value());

    std::cout << "PASS: Invalid MAC address rejection\n";
}

void testArpCacheInsertionAndLookup() {

    ArpCache cache;

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac.has_value());

    uint32_t ip = 0xC0A8010A;

    cache.addEntry(ip, *mac);

    auto result = cache.lookup(ip);

    assert(result.has_value());

    assert(*result == *mac);

    std::cout << "PASS: ARP insertion and lookup\n";
}

void testArpCacheUpdate() {

    ArpCache cache;

    uint32_t ip = 0xC0A8010A;

    auto first = MacAddress::parse("00:11:22:33:44:55");
    auto second = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(first.has_value());
    assert(second.has_value());

    cache.addEntry(ip, *first);
    cache.addEntry(ip, *second);

    auto result = cache.lookup(ip);

    assert(result.has_value());

    assert(*result == *second);

    assert(cache.getEntryCount() == 1);

    std::cout << "PASS: ARP cache update\n";
}

void testUnknownIpLookup() {

    ArpCache cache;

    auto result = cache.lookup(0xC0A80199);

    assert(!result.has_value());

    std::cout << "PASS: Unknown IP lookup\n";
}

void testArpCacheExpiration() {

    using Clock = std::chrono::steady_clock;

    ArpCache cache(std::chrono::seconds(60));

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac.has_value());

    uint32_t ip = 0xC0A8010A;

    auto start = Clock::now();

    cache.addEntry(ip, *mac, start);

    auto validResult = cache.lookup(
        ip,
        start + std::chrono::seconds(30)
    );

    assert(validResult.has_value());

    auto expiredResult = cache.lookup(
        ip,
        start + std::chrono::seconds(61)
    );

    assert(!expiredResult.has_value());

    assert(cache.getEntryCount() == 0);

    std::cout << "PASS: ARP cache expiration\n";
}

void testArpCacheRemoval() {

    ArpCache cache;

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac.has_value());

    uint32_t ip = 0xC0A8010A;

    cache.addEntry(ip, *mac);

    assert(cache.removeEntry(ip));

    assert(!cache.lookup(ip).has_value());

    std::cout << "PASS: ARP cache removal\n";
}

void testArpCacheClear() {

    ArpCache cache;

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac.has_value());

    cache.addEntry(0xC0A8010A, *mac);
    cache.addEntry(0xC0A8010B, *mac);

    cache.clear();

    assert(cache.getEntryCount() == 0);

    std::cout << "PASS: ARP cache clear\n";
}

int main() {

    testMacAddressParsing();
    testInvalidMacAddress();
    testArpCacheInsertionAndLookup();
    testArpCacheUpdate();
    testUnknownIpLookup();
    testArpCacheExpiration();
    testArpCacheRemoval();
    testArpCacheClear();

    std::cout << "\nAll ARP cache tests passed.\n";

    return 0;
}