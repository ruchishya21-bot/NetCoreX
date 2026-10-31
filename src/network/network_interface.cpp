#include "network/network_interface.hpp"

void InterfaceManager::addInterface(
    const std::string& name,
    uint32_t ipAddress
) {
    interfaces.push_back({
        name,
        ipAddress,
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