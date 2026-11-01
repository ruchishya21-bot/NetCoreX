#ifndef ICMP_HANDLER_HPP
#define ICMP_HANDLER_HPP

#include "packet/icmp.hpp"
#include "packet/packet.hpp"

#include <string>

class IcmpHandler {
public:
    IcmpError createTtlExpiredError(
        const Packet& packet,
        const std::string& routerName
    );

    IcmpError createDestinationUnreachableError(
        const Packet& packet,
        const std::string& routerName
    );
};

#endif