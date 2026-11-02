#ifndef NETWORK_INTERFACE_HPP
#define NETWORK_INTERFACE_HPP

#include <string>
#include <cstdint>
#include <vector>

#include "network/mac_address.hpp"

struct NetworkInterface {
    std::string name;
    uint32_t ipAddress;
    MacAddress macAddress;
    bool isUp;
    uint64_t packetsForwarded;
    uint64_t bytesForwarded;
};

class InterfaceManager {

private:
    std::vector<NetworkInterface> interfaces;

    static MacAddress generateMacAddress(uint32_t ipAddress);

public:
    void addInterface(
        const std::string& name,
        uint32_t ipAddress
    );

    void addInterface(
        const std::string& name,
        uint32_t ipAddress,
        const MacAddress& macAddress
    );

    NetworkInterface* findInterface(const std::string& name);

    NetworkInterface* findInterfaceByIP(uint32_t ipAddress);

    NetworkInterface* findInterfaceByMAC(
        const MacAddress& macAddress
    );

    // Read-only access to all interfaces.
    const std::vector<NetworkInterface>& getInterfaces() const {
        return interfaces;
    }
};

#endif