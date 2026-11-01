#include "nat/nat_table.hpp"

#include <cassert>
#include <iostream>

void testTCPMappingCreation() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(mapping));
    assert(table.getMappingCount() == 1);

    std::cout << "[PASS] TCP mapping creation\n";
}

void testUDPMappingCreation() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010B, 6000,
        0xCB007101, 40001,
        0x08080808, 53,
        NatProtocol::UDP
    };

    assert(table.addMapping(mapping));
    assert(table.getMappingCount() == 1);

    std::cout << "[PASS] UDP mapping creation\n";
}

void testOutboundLookup() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(mapping));

    NatLookupResult result = table.findOutbound(
        0xC0A8010A,
        5000,
        0x08080808,
        443,
        NatProtocol::TCP
    );

    assert(result.found);
    assert(result.mapping.publicIP == 0xCB007101);
    assert(result.mapping.publicPort == 40000);

    std::cout << "[PASS] Outbound mapping lookup\n";
}

void testInboundLookup() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(mapping));

    NatLookupResult result = table.findInbound(
        0xCB007101,
        40000,
        0x08080808,
        443,
        NatProtocol::TCP
    );

    assert(result.found);
    assert(result.mapping.privateIP == 0xC0A8010A);
    assert(result.mapping.privatePort == 5000);

    std::cout << "[PASS] Inbound reverse mapping lookup\n";
}

void testDuplicateOutboundMappingRejected() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(mapping));
    assert(!table.addMapping(mapping));
    assert(table.getMappingCount() == 1);

    std::cout << "[PASS] Duplicate outbound mapping rejection\n";
}

void testInboundCollisionRejected() {
    NatTable table;

    NatMapping first{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    NatMapping second{
        0xC0A8010B, 6000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(first));
    assert(!table.addMapping(second));
    assert(table.getMappingCount() == 1);

    std::cout << "[PASS] Inbound endpoint collision prevention\n";
}

void testDifferentRemoteEndpointsAllowed() {
    NatTable table;

    NatMapping first{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    NatMapping second{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x01010101, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(first));
    assert(table.addMapping(second));
    assert(table.getMappingCount() == 2);

    std::cout << "[PASS] Different remote endpoints allowed\n";
}

void testInvalidPortsRejected() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 0,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(!table.addMapping(mapping));

    mapping.privatePort = 5000;
    mapping.publicPort = 0;

    assert(!table.addMapping(mapping));

    mapping.publicPort = 40000;
    mapping.remotePort = 0;

    assert(!table.addMapping(mapping));

    assert(table.getMappingCount() == 0);

    std::cout << "[PASS] Invalid port rejection\n";
}

void testUnsupportedProtocolRejected() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        static_cast<NatProtocol>(1)
    };

    assert(!table.addMapping(mapping));

    std::cout << "[PASS] Unsupported protocol rejection\n";
}

void testMappingRemoval() {
    NatTable table;

    NatMapping mapping{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    assert(table.addMapping(mapping));

    assert(table.removeMapping(
        0xC0A8010A,
        5000,
        0x08080808,
        443,
        NatProtocol::TCP
    ));

    assert(table.getMappingCount() == 0);

    std::cout << "[PASS] Mapping removal\n";
}

void testClearMappings() {
    NatTable table;

    NatMapping first{
        0xC0A8010A, 5000,
        0xCB007101, 40000,
        0x08080808, 443,
        NatProtocol::TCP
    };

    NatMapping second{
        0xC0A8010B, 6000,
        0xCB007101, 40001,
        0x01010101, 53,
        NatProtocol::UDP
    };

    assert(table.addMapping(first));
    assert(table.addMapping(second));

    table.clear();

    assert(table.getMappingCount() == 0);

    std::cout << "[PASS] Clear all mappings\n";
}

int main() {
    testTCPMappingCreation();
    testUDPMappingCreation();
    testOutboundLookup();
    testInboundLookup();
    testDuplicateOutboundMappingRejected();
    testInboundCollisionRejected();
    testDifferentRemoteEndpointsAllowed();
    testInvalidPortsRejected();
    testUnsupportedProtocolRejected();
    testMappingRemoval();
    testClearMappings();

    std::cout << "\nAll NAT translation table tests passed!\n";

    return 0;
}