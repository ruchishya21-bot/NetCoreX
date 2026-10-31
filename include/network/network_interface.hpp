#ifndef NETWORK_INTERFACE_HPP
#define NETWORK_INTERFACE_HPP

#include <string>
#include <cstdint>
#include <vector>

struct NetworkInterface {
    std::string name;
    uint32_t ipAddress;
    bool isUp;
    uint64_t packetsForwarded;
    uint64_t bytesForwarded;
};

class InterfaceManager {

private:
    std::vector<NetworkInterface> interfaces;

public:
    void addInterface(const std::string& name, uint32_t ipAddress);

    NetworkInterface* findInterface(const std::string& name);

    NetworkInterface* findInterfaceByIP(uint32_t ipAddress);
};

#endif