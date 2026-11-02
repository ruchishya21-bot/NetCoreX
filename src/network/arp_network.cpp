#include "network/arp_network.hpp"

#include <array>
#include <utility>
#include <vector>

std::vector<uint8_t> ArpNetwork::serialize(
    const ArpMessage& message
) {

    std::vector<uint8_t> data;

    data.push_back(
        static_cast<uint8_t>(message.operation)
    );

    for (int shift = 24; shift >= 0; shift -= 8) {
        data.push_back(
            static_cast<uint8_t>(
                (message.senderIP >> shift) & 0xFF
            )
        );
    }

    const auto& senderBytes = message.senderMAC.getBytes();

    for (uint8_t byte : senderBytes) {
        data.push_back(byte);
    }

    for (int shift = 24; shift >= 0; shift -= 8) {
        data.push_back(
            static_cast<uint8_t>(
                (message.targetIP >> shift) & 0xFF
            )
        );
    }

    const auto& targetBytes = message.targetMAC.getBytes();

    for (uint8_t byte : targetBytes) {
        data.push_back(byte);
    }

    return data;
}

bool ArpNetwork::deserialize(
    const std::vector<uint8_t>& payload,
    ArpMessage& message
) {

    // 1 operation + 4 sender IP + 6 sender MAC
    // + 4 target IP + 6 target MAC = 21 bytes.
    if (payload.size() != 21) {
        return false;
    }

    if (payload[0] != 1 && payload[0] != 2) {
        return false;
    }

    message.operation =
        static_cast<ArpOperation>(payload[0]);

    message.senderIP = 0;

    for (int i = 1; i <= 4; ++i) {
        message.senderIP =
            (message.senderIP << 8) | payload[i];
    }

    std::array<uint8_t, 6> senderBytes{};

    for (int i = 0; i < 6; ++i) {
        senderBytes[i] = payload[5 + i];
    }

    message.senderMAC = MacAddress(senderBytes);

    message.targetIP = 0;

    for (int i = 11; i <= 14; ++i) {
        message.targetIP =
            (message.targetIP << 8) | payload[i];
    }

    std::array<uint8_t, 6> targetBytes{};

    for (int i = 0; i < 6; ++i) {
        targetBytes[i] = payload[15 + i];
    }

    message.targetMAC = MacAddress(targetBytes);

    return true;
}

bool ArpNetwork::addDevice(
    uint32_t ip,
    const MacAddress& mac
) {

    if (ip == 0 || devices.find(ip) != devices.end()) {
        return false;
    }

    for (const auto& entry : devices) {

        const auto& existingMAC =
            entry.second.arpEngine->getLocalMAC();

        if (existingMAC == mac) {
            return false;
        }
    }

    Device device;

    device.arpEngine =
        std::make_unique<ArpEngine>(ip, mac);

    auto result = lan.attachDevice(
        mac,
        [this, ip](const EthernetFrame& frame) {
            handleFrame(ip, frame);
        }
    );

    if (!result) {
        return false;
    }

    devices.emplace(ip, std::move(device));

    return true;
}

bool ArpNetwork::removeDevice(uint32_t ip) {

    auto it = devices.find(ip);

    if (it == devices.end()) {
        return false;
    }

    const MacAddress mac =
        it->second.arpEngine->getLocalMAC();

    lan.detachDevice(mac);

    devices.erase(it);

    return true;
}

void ArpNetwork::handleFrame(
    uint32_t deviceIP,
    const EthernetFrame& frame
) {

    if (frame.getEtherType() != EtherType::ARP) {
        return;
    }

    auto deviceIt = devices.find(deviceIP);

    if (deviceIt == devices.end()) {
        return;
    }

    ArpMessage message;

    if (!deserialize(frame.getPayload(), message)) {
        return;
    }

    ArpEngine& engine =
        *deviceIt->second.arpEngine;

    if (message.operation == ArpOperation::Request) {

        auto reply = engine.processRequest(message);

        if (!reply.has_value()) {
            return;
        }

        EthernetFrame replyFrame(
            engine.getLocalMAC(),
            message.senderMAC,
            EtherType::ARP,
            serialize(*reply)
        );

        lan.transmit(replyFrame);
    }

    else if (message.operation == ArpOperation::Reply) {

        engine.processReply(message);
    }
}

bool ArpNetwork::requestResolution(
    uint32_t senderIP,
    uint32_t targetIP
) {

    auto sender = devices.find(senderIP);

    if (sender == devices.end()) {
        return false;
    }

    ArpEngine& engine =
        *sender->second.arpEngine;

    // Avoid broadcasting if the mapping is already cached.
    if (engine.resolve(targetIP).has_value()) {
        return true;
    }

    ArpMessage request =
        engine.createRequest(targetIP);

    EthernetFrame requestFrame(
        engine.getLocalMAC(),
        EthernetFrame::broadcastMAC(),
        EtherType::ARP,
        serialize(request)
    );

    lan.transmit(requestFrame);

    return true;
}

std::optional<MacAddress> ArpNetwork::resolve(
    uint32_t deviceIP,
    uint32_t targetIP
) {

    auto it = devices.find(deviceIP);

    if (it == devices.end()) {
        return std::nullopt;
    }

    return it->second.arpEngine->resolve(targetIP);
}

std::size_t ArpNetwork::getDeviceCount() const {
    return devices.size();
}

const LanSegment& ArpNetwork::getLan() const {
    return lan;
}