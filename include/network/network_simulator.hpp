#ifndef NETWORK_SIMULATOR_HPP
#define NETWORK_SIMULATOR_HPP

#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <cstdint>

#include "network/router.hpp"
#include "network/lan_segment.hpp"
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

struct ReceivedEthernetFrame {
    Router* router;
    std::string interfaceName;
    EthernetFrame frame;
};

class NetworkSimulator {

private:

    struct InterfaceLink {
        Router* firstRouter;
        std::string firstInterface;

        Router* secondRouter;
        std::string secondInterface;

        std::shared_ptr<LanSegment> lan;
    };

    std::vector<Router*> routers;

    std::vector<InterfaceLink> interfaceLinks;

    std::vector<ReceivedEthernetFrame> receivedFrames;

    IcmpHandler icmpHandler;

    std::optional<IcmpError> lastIcmpError;

    // ARP message serialization.
    static std::vector<uint8_t> serializeArp(
        const ArpMessage& message
    );

    static bool deserializeArp(
        const std::vector<uint8_t>& payload,
        ArpMessage& message
    );

    // Resolve a next-hop MAC using ARP over Ethernet.
    std::optional<MacAddress> resolveNextHopMac(
        Router& router,
        const std::string& interfaceName,
        uint32_t nextHopIP
    );

    // Process received ARP frames.
    void processArpFrames();

public:

    void addRouter(Router& router);

    bool connectInterfaces(
        Router& firstRouter,
        const std::string& firstInterface,
        Router& secondRouter,
        const std::string& secondInterface
    );

    bool transmitFrame(
        Router& router,
        const std::string& interfaceName,
        const EthernetFrame& frame
    );

    std::vector<ReceivedEthernetFrame> takeReceivedFrames();

    std::size_t getLinkCount() const;

    ForwardingResult simulate(
        Packet& packet,
        Router& sourceRouter
    );

    const std::optional<IcmpError>& getLastIcmpError() const;
};

#endif