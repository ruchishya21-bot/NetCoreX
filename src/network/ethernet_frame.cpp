#include "network/ethernet_frame.hpp"

#include <stdexcept>

EthernetFrame::EthernetFrame(
    const MacAddress& source,
    const MacAddress& destination,
    EtherType type,
    const std::vector<uint8_t>& data
)
    : destinationMAC(destination),
      sourceMAC(source),
      etherType(type),
      payload(data) {

    if (payload.size() > MAX_PAYLOAD_SIZE) {
        throw std::invalid_argument(
            "Ethernet payload exceeds maximum size"
        );
    }
}

const MacAddress& EthernetFrame::getSourceMAC() const {
    return sourceMAC;
}

const MacAddress& EthernetFrame::getDestinationMAC() const {
    return destinationMAC;
}

EtherType EthernetFrame::getEtherType() const {
    return etherType;
}

const std::vector<uint8_t>& EthernetFrame::getPayload() const {
    return payload;
}

MacAddress EthernetFrame::broadcastMAC() {
    auto address = MacAddress::parse("FF:FF:FF:FF:FF:FF");

    return address.value();
}

bool EthernetFrame::isBroadcast() const {
    return destinationMAC == broadcastMAC();
}

bool EthernetFrame::isValid() const {
    return sourceMAC != broadcastMAC()
        && payload.size() <= MAX_PAYLOAD_SIZE
        && (
            etherType == EtherType::IPv4 ||
            etherType == EtherType::ARP
        );
}