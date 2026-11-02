#ifndef NETWORK_SIMULATOR_HPP
#define NETWORK_SIMULATOR_HPP

#include <vector>
#include <optional>

#include "network/router.hpp"
#include "packet/packet.hpp"
#include "packet/icmp_handler.hpp"

enum class ForwardingResult {
    Delivered,
    TtlExpired,
    NoRoute,
    InterfaceDown,
    NextHopUnreachable,
    RoutingLoop,
    HopLimitExceeded,
    AclDenied,
    StatefulFirewallDenied,
    NatTranslationFailed
};

class NetworkSimulator {

private:
    std::vector<Router*> routers;

    IcmpHandler icmpHandler;

    std::optional<IcmpError> lastIcmpError;

public:
    void addRouter(Router& router);

    ForwardingResult simulate(
        Packet& packet,
        Router& sourceRouter
    );

    const std::optional<IcmpError>& getLastIcmpError() const;
};

#endif