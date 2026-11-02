#include "network/network_interface.hpp"

MacAddress InterfaceManager::generateMacAddress(
    uint32_t ipAddress
) {

    std::array<uint8_t, 6> bytes{
        0x02,
        0x00,
        static_cast<uint8_t>((ipAddress >> 24) & 0xFF),
        static_cast<uint8_t>((ipAddress >> 16) & 0xFF),
        static_cast<uint8_t>((ipAddress >> 8) & 0xFF),
        static_cast<uint8_t>(ipAddress & 0xFF)
    };

    return MacAddress(bytes);
}

void InterfaceManager::addInterface(
    const std::string& name,
    uint32_t ipAddress
) {

    addInterface(
        name,
        ipAddress,
        generateMacAddress(ipAddress)
    );
}

void InterfaceManager::addInterface(
    const std::string& name,
    uint32_t ipAddress,
    const MacAddress& macAddress
) {

    interfaces.push_back({
        name,
        ipAddress,
        macAddress,
        true,
        0,
        0
    });
}

NetworkInterface* InterfaceManager::findInterface(
    const std::string& name
) {

    for (auto& interface : interfaces) {

        if (interface.name == name) {
            return &interface;
        }
    }

    return nullptr;
}

NetworkInterface* InterfaceManager::findInterfaceByIP(
    uint32_t ipAddress
) {

    for (auto& interface : interfaces) {

        if (interface.ipAddress == ipAddress) {
            return &interface;
        }
    }

    return nullptr;
}

NetworkInterface* InterfaceManager::findInterfaceByMAC(
    const MacAddress& macAddress
) {

    for (auto& interface : interfaces) {

        if (interface.macAddress == macAddress) {
            return &interface;
        }
    }

    return nullptr;
}