#ifndef ROUTE_TABLE_HPP
#define ROUTE_TABLE_HPP

#include <cstdint>
#include <string>
#include <vector>

struct Route {
    uint32_t network;
    uint8_t prefixLength;
    uint32_t nextHop;
    std::string interfaceName;
    bool dynamic = false;
};

class RouteTable {

private:
    std::vector<Route> routes;

public:
    void addRoute(
        uint32_t network,
        uint8_t prefixLength,
        uint32_t nextHop,
        const std::string& interfaceName
    );

    void addDynamicRoute(
        uint32_t network,
        uint8_t prefixLength,
        uint32_t nextHop,
        const std::string& interfaceName
    );

    void clearDynamicRoutes();

    const Route* findRoute(uint32_t destinationIP) const;

    // Read-only access to all routing entries.
    const std::vector<Route>& getRoutes() const {
        return routes;
    }
};

#endif