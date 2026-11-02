#ifndef LAN_SEGMENT_HPP
#define LAN_SEGMENT_HPP

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "network/ethernet_frame.hpp"

class LanSegment {

public:
    using FrameHandler = std::function<void(const EthernetFrame&)>;

private:
    std::vector<std::pair<MacAddress, FrameHandler>> devices;

public:
    bool attachDevice(
        const MacAddress& mac,
        FrameHandler handler
    );

    bool detachDevice(const MacAddress& mac);

    std::size_t transmit(const EthernetFrame& frame);

    std::size_t getDeviceCount() const;

    bool hasDevice(const MacAddress& mac) const;
};

#endif