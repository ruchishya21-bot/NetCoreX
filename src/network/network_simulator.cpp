#include "network/network_simulator.hpp"
#include "forwarding/forwarding_decision.hpp"
#include "packet/ipv4_codec.hpp"

#include <iostream>
#include <unordered_set>
#include <utility>
#include <sstream>
#include <iomanip>
#include <vector>

namespace {

constexpr uint8_t TCP_FIN = 0x01;
constexpr uint8_t TCP_SYN = 0x02;
constexpr uint8_t TCP_RST = 0x04;
constexpr uint8_t TCP_ACK = 0x10;

bool isInitialTcpSyn(const Packet& packet) {

    const bool syn = (packet.tcpFlags & TCP_SYN) != 0;
    const bool ack = (packet.tcpFlags & TCP_ACK) != 0;
    const bool fin = (packet.tcpFlags & TCP_FIN) != 0;
    const bool rst = (packet.tcpFlags & TCP_RST) != 0;

    return syn && !ack && !fin && !rst;
}

bool isNatSupportedProtocol(const Packet& packet) {
    return packet.protocol == 6 || packet.protocol == 17;
}

std::optional<MacAddress> macFromBytes(
    const std::vector<uint8_t>& bytes,
    std::size_t offset
) {

    if (offset + 6 > bytes.size()) {
        return std::nullopt;
    }

    std::ostringstream stream;

    stream << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < 6; ++i) {

        if (i != 0) {
            stream << ":";
        }

        stream << std::setw(2)
               << static_cast<unsigned int>(bytes[offset + i]);
    }

    return MacAddress::parse(stream.str());
}

}

// ======================================================
// ROUTER REGISTRATION
// ======================================================

void NetworkSimulator::addRouter(Router& router) {

    for (Router* existing : routers) {

        if (existing == &router) {
            return;
        }
    }

    routers.push_back(&router);
}

// ======================================================
// ARP SERIALIZATION
// ======================================================

std::vector<uint8_t> NetworkSimulator::serializeArp(
    const ArpMessage& message
) {

    std::vector<uint8_t> payload;

    payload.reserve(21);

    payload.push_back(
        static_cast<uint8_t>(message.operation)
    );

    // Sender IP.
    payload.push_back(
        static_cast<uint8_t>(message.senderIP >> 24)
    );

    payload.push_back(
        static_cast<uint8_t>(message.senderIP >> 16)
    );

    payload.push_back(
        static_cast<uint8_t>(message.senderIP >> 8)
    );

    payload.push_back(
        static_cast<uint8_t>(message.senderIP)
    );

    // Sender MAC.
    const auto& senderBytes = message.senderMAC.getBytes();

    for (uint8_t byte : senderBytes) {
        payload.push_back(byte);
    }

    // Target IP.
    payload.push_back(
        static_cast<uint8_t>(message.targetIP >> 24)
    );

    payload.push_back(
        static_cast<uint8_t>(message.targetIP >> 16)
    );

    payload.push_back(
        static_cast<uint8_t>(message.targetIP >> 8)
    );

    payload.push_back(
        static_cast<uint8_t>(message.targetIP)
    );

    // Target MAC.
    const auto& targetBytes = message.targetMAC.getBytes();

    for (uint8_t byte : targetBytes) {
        payload.push_back(byte);
    }

    return payload;
}

// ======================================================
// ARP DESERIALIZATION
// ======================================================

bool NetworkSimulator::deserializeArp(
    const std::vector<uint8_t>& payload,
    ArpMessage& message
) {

    if (payload.size() != 21) {
        return false;
    }

    if (payload[0] != 1 && payload[0] != 2) {
        return false;
    }

    message.operation =
        static_cast<ArpOperation>(payload[0]);

    message.senderIP =
        (static_cast<uint32_t>(payload[1]) << 24) |
        (static_cast<uint32_t>(payload[2]) << 16) |
        (static_cast<uint32_t>(payload[3]) << 8) |
        static_cast<uint32_t>(payload[4]);

    auto senderMAC = macFromBytes(payload, 5);

    if (!senderMAC.has_value()) {
        return false;
    }

    message.senderMAC = *senderMAC;

    message.targetIP =
        (static_cast<uint32_t>(payload[11]) << 24) |
        (static_cast<uint32_t>(payload[12]) << 16) |
        (static_cast<uint32_t>(payload[13]) << 8) |
        static_cast<uint32_t>(payload[14]);

    auto targetMAC = macFromBytes(payload, 15);

    if (!targetMAC.has_value()) {
        return false;
    }

    message.targetMAC = *targetMAC;

    return true;
}

// ======================================================
// ETHERNET LINK CONNECTION
// ======================================================

bool NetworkSimulator::connectInterfaces(
    Router& firstRouter,
    const std::string& firstInterfaceName,
    Router& secondRouter,
    const std::string& secondInterfaceName
) {

    if (&firstRouter == &secondRouter) {
        return false;
    }

    auto* firstInterface =
        firstRouter.getInterfaceManager()
                   .findInterface(firstInterfaceName);

    auto* secondInterface =
        secondRouter.getInterfaceManager()
                    .findInterface(secondInterfaceName);

    if (
        firstInterface == nullptr ||
        secondInterface == nullptr
    ) {
        return false;
    }

    if (
        !firstInterface->isUp ||
        !secondInterface->isUp
    ) {
        return false;
    }

    // Ensure neither interface is already connected.
    for (const auto& link : interfaceLinks) {

        if (
            (link.firstRouter == &firstRouter &&
             link.firstInterface == firstInterfaceName) ||

            (link.secondRouter == &firstRouter &&
             link.secondInterface == firstInterfaceName) ||

            (link.firstRouter == &secondRouter &&
             link.firstInterface == secondInterfaceName) ||

            (link.secondRouter == &secondRouter &&
             link.secondInterface == secondInterfaceName)
        ) {
            return false;
        }
    }

    auto lan = std::make_shared<LanSegment>();

    const MacAddress firstMAC =
        firstInterface->macAddress;

    const MacAddress secondMAC =
        secondInterface->macAddress;

    bool firstAttached = lan->attachDevice(
        firstMAC,
        [this, &firstRouter, firstInterfaceName](
            const EthernetFrame& frame
        ) {

            receivedFrames.push_back({
                &firstRouter,
                firstInterfaceName,
                frame
            });
        }
    );

    if (!firstAttached) {
        return false;
    }

    bool secondAttached = lan->attachDevice(
        secondMAC,
        [this, &secondRouter, secondInterfaceName](
            const EthernetFrame& frame
        ) {

            receivedFrames.push_back({
                &secondRouter,
                secondInterfaceName,
                frame
            });
        }
    );

    if (!secondAttached) {
        return false;
    }

    interfaceLinks.push_back({
        &firstRouter,
        firstInterfaceName,
        &secondRouter,
        secondInterfaceName,
        std::move(lan)
    });

    return true;
}

// ======================================================
// ETHERNET FRAME TRANSMISSION
// ======================================================

bool NetworkSimulator::transmitFrame(
    Router& router,
    const std::string& interfaceName,
    const EthernetFrame& frame
) {

    auto* interface =
        router.getInterfaceManager()
              .findInterface(interfaceName);

    if (
        interface == nullptr ||
        !interface->isUp
    ) {
        return false;
    }

    if (frame.getSourceMAC() != interface->macAddress) {
        return false;
    }

    if (!frame.isValid()) {
        return false;
    }

    for (auto& link : interfaceLinks) {

        bool connected =
            (link.firstRouter == &router &&
             link.firstInterface == interfaceName) ||

            (link.secondRouter == &router &&
             link.secondInterface == interfaceName);

        if (connected) {
            return link.lan->transmit(frame) > 0;
        }
    }

    return false;
}

// ======================================================
// RETRIEVE RECEIVED ETHERNET FRAMES
// ======================================================

std::vector<ReceivedEthernetFrame>
NetworkSimulator::takeReceivedFrames() {

    std::vector<ReceivedEthernetFrame> result;

    result.swap(receivedFrames);

    return result;
}

// ======================================================
// GET LINK COUNT
// ======================================================

std::size_t NetworkSimulator::getLinkCount() const {
    return interfaceLinks.size();
}

// ======================================================
// PROCESS ARP FRAMES
// ======================================================

void NetworkSimulator::processArpFrames() {

    while (!receivedFrames.empty()) {

        auto frames = takeReceivedFrames();

        for (const auto& received : frames) {

            if (received.router == nullptr) {
                continue;
            }

            const EthernetFrame& frame = received.frame;

            if (frame.getEtherType() != EtherType::ARP) {
                continue;
            }

            ArpMessage message;

            if (!deserializeArp(frame.getPayload(), message)) {
                continue;
            }

            ArpEngine* engine =
                received.router->getArpEngine(
                    received.interfaceName
                );

            if (engine == nullptr) {
                continue;
            }

            // Handle ARP request.
            if (message.operation == ArpOperation::Request) {

                auto reply = engine->processRequest(message);

                if (!reply.has_value()) {
                    continue;
                }

                auto* interface =
                    received.router->getInterfaceManager()
                                    .findInterface(
                                        received.interfaceName
                                    );

                if (
                    interface == nullptr ||
                    !interface->isUp
                ) {
                    continue;
                }

                EthernetFrame replyFrame(
                    interface->macAddress,
                    message.senderMAC,
                    EtherType::ARP,
                    serializeArp(*reply)
                );

                transmitFrame(
                    *received.router,
                    received.interfaceName,
                    replyFrame
                );
            }

            // Handle ARP reply.
            else if (message.operation == ArpOperation::Reply) {

                engine->processReply(message);
            }
        }
    }
}

// ======================================================
// NEXT-HOP MAC RESOLUTION
// ======================================================

std::optional<MacAddress>
NetworkSimulator::resolveNextHopMac(
    Router& router,
    const std::string& interfaceName,
    uint32_t nextHopIP
) {

    ArpEngine* engine =
        router.getArpEngine(interfaceName);

    if (engine == nullptr) {
        return std::nullopt;
    }

    // Check ARP cache first.
    auto cachedMAC = engine->resolve(nextHopIP);

    if (cachedMAC.has_value()) {
        return cachedMAC;
    }

    auto* interface =
        router.getInterfaceManager()
              .findInterface(interfaceName);

    if (
        interface == nullptr ||
        !interface->isUp
    ) {
        return std::nullopt;
    }

    // Create ARP request.
    ArpMessage request =
        engine->createRequest(nextHopIP);

    EthernetFrame requestFrame(
        interface->macAddress,
        EthernetFrame::broadcastMAC(),
        EtherType::ARP,
        serializeArp(request)
    );

    std::cout
        << "ARP: Resolving next-hop IP on "
        << router.getName()
        << " interface "
        << interfaceName << "\n";

    // Broadcast ARP request.
    if (
        !transmitFrame(
            router,
            interfaceName,
            requestFrame
        )
    ) {

        std::cout << "ARP: Request transmission failed.\n";

        return std::nullopt;
    }

    // Process request and reply.
    processArpFrames();

    // Check cache again.
    auto resolvedMAC = engine->resolve(nextHopIP);

    if (!resolvedMAC.has_value()) {

        std::cout << "ARP: Next-hop resolution failed.\n";

        return std::nullopt;
    }

    std::cout << "ARP: Next-hop MAC resolved successfully.\n";

    return resolvedMAC;
}

// ======================================================
// PACKET FORWARDING
// ======================================================

ForwardingResult NetworkSimulator::simulate(
    Packet& packet,
    Router& sourceRouter
) {

    lastIcmpError.reset();

    Router* currentRouter = &sourceRouter;

    ForwardingDecisionEngine decisionEngine;

    std::unordered_set<std::string> visited;

    constexpr int MAX_HOPS = 16;

    int hops = 0;

    bool inboundNatProcessed = false;
    bool outboundNatProcessed = false;

    std::cout << "\n========== PACKET FORWARDING ==========\n";

    std::cout << "Starting router: "
              << currentRouter->getName() << "\n";

    std::cout << "Initial TTL: "
              << static_cast<int>(packet.ttl) << "\n";

    // ==========================================
    // INBOUND DESTINATION NAT
    // ==========================================

    if (sourceRouter.isNatEnabled()) {

        NatEngine* nat =
            sourceRouter.getNatEngine();

        if (
            nat != nullptr &&
            packet.destinationIP == nat->getPublicIP() &&
            isNatSupportedProtocol(packet)
        ) {

            std::cout << "NAT: Processing inbound packet.\n";

            if (!nat->translateInbound(packet)) {

                std::cout
                    << "NAT DROP: No matching inbound translation.\n";

                return ForwardingResult::NatTranslationFailed;
            }

            inboundNatProcessed = true;

            std::cout << "NAT: Inbound destination translated.\n";
        }
    }

    // ==========================================
    // ROUTER FORWARDING LOOP
    // ==========================================

    while (hops < MAX_HOPS) {

        // Loop detection.
        if (!visited.insert(currentRouter->getName()).second) {

            std::cout << "DROP: Routing loop detected.\n";

            return ForwardingResult::RoutingLoop;
        }

        // ==========================================
        // STATEFUL FIREWALL
        // ==========================================

        if (currentRouter->isStatefulFirewallEnabled()) {

            if (packet.protocol == 6) {

                ConnectionTracker& tracker =
                    currentRouter->getConnectionTracker();

                ConnectionState existingState;

                const bool connectionExists =
                    tracker.getConnectionState(
                        packet,
                        existingState
                    );

                if (connectionExists) {

                    ConnectionUpdateResult update =
                        tracker.processPacket(packet);

                    if (
                        update.status ==
                            ConnectionStatus::InvalidTransition ||
                        update.status ==
                            ConnectionStatus::UnsupportedProtocol
                    ) {

                        std::cout
                            << "STATEFUL FIREWALL: Packet rejected at "
                            << currentRouter->getName()
                            << "\n";

                        std::cout << update.message << "\n";

                        return ForwardingResult::StatefulFirewallDenied;
                    }

                    std::cout
                        << "STATEFUL FIREWALL: Existing connection accepted. "
                        << update.message << "\n";
                }

                else {

                    if (!isInitialTcpSyn(packet)) {

                        std::cout
                            << "STATEFUL FIREWALL: Unknown TCP packet rejected.\n";

                        return ForwardingResult::StatefulFirewallDenied;
                    }

                    if (currentRouter->isAccessControlEnabled()) {

                        ACLDecision aclDecision =
                            currentRouter->getAccessControlList()
                                         .evaluate(packet);

                        if (aclDecision.action == ACLAction::Deny) {

                            std::cout
                                << "STATEFUL FIREWALL: New connection rejected by ACL.\n";

                            std::cout << aclDecision.message << "\n";

                            return ForwardingResult::AclDenied;
                        }
                    }

                    ConnectionUpdateResult update =
                        tracker.processPacket(packet);

                    if (
                        update.status !=
                        ConnectionStatus::NewConnection
                    ) {

                        std::cout
                            << "STATEFUL FIREWALL: Unable to create connection.\n";

                        std::cout << update.message << "\n";

                        return ForwardingResult::StatefulFirewallDenied;
                    }

                    std::cout
                        << "STATEFUL FIREWALL: New TCP connection accepted.\n";
                }
            }

            else {

                if (currentRouter->isAccessControlEnabled()) {

                    ACLDecision aclDecision =
                        currentRouter->getAccessControlList()
                                     .evaluate(packet);

                    if (aclDecision.action == ACLAction::Deny) {

                        std::cout
                            << "DROP: ACL denied non-TCP packet at "
                            << currentRouter->getName() << "\n";

                        std::cout << aclDecision.message << "\n";

                        return ForwardingResult::AclDenied;
                    }
                }
            }
        }

        // ==========================================
        // STATELESS ACL
        // ==========================================

        else if (currentRouter->isAccessControlEnabled()) {

            ACLDecision aclDecision =
                currentRouter->getAccessControlList()
                             .evaluate(packet);

            if (aclDecision.action == ACLAction::Deny) {

                std::cout
                    << "DROP: ACL denied packet at "
                    << currentRouter->getName() << "\n";

                std::cout << aclDecision.message << "\n";

                return ForwardingResult::AclDenied;
            }
        }

        // ==========================================
        // OUTBOUND SOURCE NAT
        // ==========================================

        if (
            currentRouter == &sourceRouter &&
            !outboundNatProcessed &&
            !inboundNatProcessed &&
            currentRouter->isNatEnabled() &&
            isNatSupportedProtocol(packet)
        ) {

            NatEngine* nat =
                currentRouter->getNatEngine();

            if (
                nat != nullptr &&
                packet.sourceIP != nat->getPublicIP()
            ) {

                std::cout << "NAT: Processing outbound packet.\n";

                if (!nat->translateOutbound(packet)) {

                    std::cout
                        << "NAT DROP: Outbound translation failed.\n";

                    return ForwardingResult::NatTranslationFailed;
                }

                std::cout
                    << "NAT: Outbound source translated successfully.\n";
            }

            outboundNatProcessed = true;
        }

        // ==========================================
        // TTL CHECK
        // ==========================================

        if (packet.ttl <= 1) {

            lastIcmpError =
                icmpHandler.createTtlExpiredError(
                    packet,
                    currentRouter->getName()
                );

            std::cout
                << "DROP: IPv4 TTL expired at "
                << currentRouter->getName() << "\n";

            std::cout << lastIcmpError->message << "\n";

            return ForwardingResult::TtlExpired;
        }

        // ==========================================
        // ROUTING DECISION
        // ==========================================

        ForwardingDecision decision =
            decisionEngine.evaluate(
                packet,
                currentRouter->getRoutingTable(),
                currentRouter->getInterfaceManager()
            );

        if (decision.status == ForwardingStatus::NoRoute) {

            lastIcmpError =
                icmpHandler.createDestinationUnreachableError(
                    packet,
                    currentRouter->getName()
                );

            std::cout
                << "DROP: No matching route at "
                << currentRouter->getName() << "\n";

            std::cout << lastIcmpError->message << "\n";

            return ForwardingResult::NoRoute;
        }

        if (
            decision.status ==
            ForwardingStatus::InterfaceUnavailable
        ) {

            std::cout
                << "DROP: Outgoing interface unavailable at "
                << currentRouter->getName() << "\n";

            return ForwardingResult::InterfaceDown;
        }

        auto* outgoingInterface =
            currentRouter->getInterfaceManager()
                         .findInterface(decision.interfaceName);

        if (outgoingInterface == nullptr) {
            return ForwardingResult::InterfaceDown;
        }

        // ==========================================
        // DIRECTLY CONNECTED DESTINATION
        // ==========================================

        if (decision.nextHop == 0) {

            packet.ttl--;

            outgoingInterface->packetsForwarded++;

            outgoingInterface->bytesForwarded +=
                packet.payload.size();

            std::cout
                << currentRouter->getName()
                << " -> interface "
                << outgoingInterface->name
                << " | TTL: "
                << static_cast<int>(packet.ttl)
                << "\n";

            std::cout << "PACKET DELIVERED\n";

            std::cout << "Total hops: "
                      << hops + 1 << "\n";

            std::cout << "Remaining TTL: "
                      << static_cast<int>(packet.ttl) << "\n";

            std::cout
                << "=======================================\n";

            return ForwardingResult::Delivered;
        }

        // ==========================================
        // ARP NEXT-HOP RESOLUTION
        // ==========================================

        auto nextHopMAC =
            resolveNextHopMac(
                *currentRouter,
                decision.interfaceName,
                decision.nextHop
            );

        if (!nextHopMAC.has_value()) {

            std::cout
                << "DROP: Unable to resolve next-hop MAC.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        // ==========================================
        // IDENTIFY NEXT ROUTER AND INTERFACE
        // ==========================================

        Router* nextRouter = nullptr;

        NetworkInterface* nextInterface = nullptr;

        for (Router* candidate : routers) {

            if (candidate == currentRouter) {
                continue;
            }

            auto* candidateInterface =
                candidate->getInterfaceManager()
                         .findInterfaceByMAC(*nextHopMAC);

            if (
                candidateInterface != nullptr &&
                candidateInterface->isUp
            ) {

                nextRouter = candidate;

                nextInterface = candidateInterface;

                break;
            }
        }

        if (
            nextRouter == nullptr ||
            nextInterface == nullptr
        ) {

            std::cout
                << "DROP: Resolved MAC does not belong to a registered router.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        // ==========================================
        // DECREMENT TTL
        // ==========================================

        packet.ttl--;

        std::cout
            << currentRouter->getName()
            << " -> interface "
            << outgoingInterface->name
            << " | TTL: "
            << static_cast<int>(packet.ttl)
            << "\n";

        // ==========================================
        // IPV4 SERIALIZATION
        // ==========================================

        std::vector<uint8_t> ipv4Payload;

        if (
            !IPv4PacketCodec::serialize(
                packet,
                ipv4Payload
            )
        ) {

            std::cout
                << "DROP: IPv4 packet serialization failed.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        // ==========================================
        // ETHERNET ENCAPSULATION
        // ==========================================

        EthernetFrame outgoingFrame(
            outgoingInterface->macAddress,
            *nextHopMAC,
            EtherType::IPv4,
            ipv4Payload
        );

        if (!outgoingFrame.isValid()) {

            std::cout
                << "DROP: Invalid outgoing Ethernet frame.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        // ==========================================
        // ETHERNET TRANSMISSION
        // ==========================================

        std::cout
            << "ETHERNET TX: "
            << currentRouter->getName()
            << " -> "
            << nextRouter->getName()
            << "\n";

        if (
            !transmitFrame(
                *currentRouter,
                outgoingInterface->name,
                outgoingFrame
            )
        ) {

            std::cout
                << "DROP: Ethernet frame transmission failed.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        outgoingInterface->packetsForwarded++;

        outgoingInterface->bytesForwarded +=
            ipv4Payload.size();

        // ==========================================
        // ETHERNET RECEPTION
        // ==========================================

        std::vector<ReceivedEthernetFrame> incomingFrames =
            takeReceivedFrames();

        bool frameReceived = false;

        Packet receivedPacket;

        for (const auto& received : incomingFrames) {

            if (
                received.router != nextRouter ||
                received.interfaceName != nextInterface->name
            ) {
                continue;
            }

            const EthernetFrame& incomingFrame =
                received.frame;

            // Validate Ethernet frame.
            if (!incomingFrame.isValid()) {
                continue;
            }

            // Validate EtherType.
            if (
                incomingFrame.getEtherType() !=
                EtherType::IPv4
            ) {
                continue;
            }

            // Validate destination MAC.
            if (
                incomingFrame.getDestinationMAC() !=
                nextInterface->macAddress
            ) {
                continue;
            }

            // Validate source MAC.
            if (
                incomingFrame.getSourceMAC() !=
                outgoingInterface->macAddress
            ) {
                continue;
            }

            // ======================================
            // IPV4 DECAPSULATION
            // ======================================

            if (
                !IPv4PacketCodec::deserialize(
                    incomingFrame.getPayload(),
                    receivedPacket
                )
            ) {
                continue;
            }

            frameReceived = true;

            break;
        }

        if (!frameReceived) {

            std::cout
                << "DROP: Ethernet frame was not received or failed validation.\n";

            return ForwardingResult::NextHopUnreachable;
        }

        std::cout
            << "ETHERNET RX: "
            << nextRouter->getName()
            << " | Frame validated and IPv4 packet reconstructed.\n";

        // ==========================================
        // MOVE RECONSTRUCTED PACKET TO NEXT ROUTER
        // ==========================================

        packet = std::move(receivedPacket);

        currentRouter = nextRouter;

        hops++;
    }

    std::cout
        << "DROP: Maximum hop limit exceeded.\n";

    return ForwardingResult::HopLimitExceeded;
}

// ======================================================
// LAST ICMP ERROR
// ======================================================

const std::optional<IcmpError>&
NetworkSimulator::getLastIcmpError() const {

    return lastIcmpError;
}