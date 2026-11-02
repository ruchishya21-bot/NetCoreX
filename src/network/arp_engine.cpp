#include "network/arp_engine.hpp"

ArpEngine::ArpEngine(
    uint32_t ipAddress,
    const MacAddress& macAddress
)
    : localIP(ipAddress),
      localMAC(macAddress) {
}

ArpMessage ArpEngine::createRequest(
    uint32_t targetIP
) const {

    ArpMessage request;

    request.operation = ArpOperation::Request;

    request.senderIP = localIP;
    request.senderMAC = localMAC;

    request.targetIP = targetIP;

    // An ARP request does not initially know
    // the target's MAC address.
    request.targetMAC = MacAddress();

    return request;
}

std::optional<ArpMessage> ArpEngine::processRequest(
    const ArpMessage& request
) {

    if (request.operation != ArpOperation::Request) {
        return std::nullopt;
    }

    if (request.senderIP == 0 || request.targetIP == 0) {
        return std::nullopt;
    }

    // Learn the sender's IP-to-MAC mapping.
    arpCache.addEntry(
        request.senderIP,
        request.senderMAC
    );

    // Only the device owning the requested IP replies.
    if (request.targetIP != localIP) {
        return std::nullopt;
    }

    ArpMessage reply;

    reply.operation = ArpOperation::Reply;

    reply.senderIP = localIP;
    reply.senderMAC = localMAC;

    reply.targetIP = request.senderIP;
    reply.targetMAC = request.senderMAC;

    return reply;
}

bool ArpEngine::processReply(
    const ArpMessage& reply
) {

    if (reply.operation != ArpOperation::Reply) {
        return false;
    }

    if (reply.senderIP == 0 || reply.targetIP != localIP) {
        return false;
    }

    // Learn the responding device's MAC address.
    arpCache.addEntry(
        reply.senderIP,
        reply.senderMAC
    );

    return true;
}

std::optional<MacAddress> ArpEngine::resolve(
    uint32_t ipAddress
) {

    return arpCache.lookup(ipAddress);
}

ArpCache& ArpEngine::getArpCache() {
    return arpCache;
}

const ArpCache& ArpEngine::getArpCache() const {
    return arpCache;
}

uint32_t ArpEngine::getLocalIP() const {
    return localIP;
}

const MacAddress& ArpEngine::getLocalMAC() const {
    return localMAC;
}