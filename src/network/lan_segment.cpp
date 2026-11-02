#include "network/lan_segment.hpp"

#include <stdexcept>

bool LanSegment::attachDevice(
    const MacAddress& mac,
    FrameHandler handler
) {
    if (!handler) {
        return false;
    }

    if (hasDevice(mac)) {
        return false;
    }

    devices.emplace_back(mac, std::move(handler));

    return true;
}

bool LanSegment::detachDevice(const MacAddress& mac) {

    for (auto it = devices.begin(); it != devices.end(); ++it) {

        if (it->first == mac) {
            devices.erase(it);
            return true;
        }
    }

    return false;
}

std::size_t LanSegment::transmit(const EthernetFrame& frame) {

    if (!frame.isValid()) {
        return 0;
    }

    // The transmitting device must belong to this LAN.
    if (!hasDevice(frame.getSourceMAC())) {
        return 0;
    }

    std::vector<FrameHandler> recipients;

    if (frame.isBroadcast()) {

        // Broadcast to every device except the sender.
        for (const auto& device : devices) {

            if (device.first != frame.getSourceMAC()) {
                recipients.push_back(device.second);
            }
        }

    } else {

        // Unicast to the matching destination MAC.
        for (const auto& device : devices) {

            if (device.first == frame.getDestinationMAC() &&
                device.first != frame.getSourceMAC()) {

                recipients.push_back(device.second);
                break;
            }
        }
    }

    // Copy callbacks before invoking them.
    // This prevents iterator invalidation if a handler
    // changes device registration.
    for (const auto& handler : recipients) {
        handler(frame);
    }

    return recipients.size();
}

std::size_t LanSegment::getDeviceCount() const {
    return devices.size();
}

bool LanSegment::hasDevice(const MacAddress& mac) const {

    for (const auto& device : devices) {

        if (device.first == mac) {
            return true;
        }
    }

    return false;
}