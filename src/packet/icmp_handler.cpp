#include "packet/icmp_handler.hpp"

IcmpError IcmpHandler::createTtlExpiredError(
    const Packet& packet,
    const std::string& routerName
) {
    return {
        IcmpType::TimeExceeded,
        IcmpCode::TtlExpired,
        routerName,
        packet.sourceIP,
        packet.destinationIP,
        packet.packetId,
        "ICMP Time Exceeded: IPv4 TTL expired"
    };
}

IcmpError IcmpHandler::createDestinationUnreachableError(
    const Packet& packet,
    const std::string& routerName
) {
    return {
        IcmpType::DestinationUnreachable,
        IcmpCode::NetworkUnreachable,
        routerName,
        packet.sourceIP,
        packet.destinationIP,
        packet.packetId,
        "ICMP Destination Unreachable: No matching route"
    };
}