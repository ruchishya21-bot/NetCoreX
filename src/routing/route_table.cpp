#include "routing/route_table.hpp"

#include <algorithm>

void RouteTable::addRoute(
    uint32_t network,
    uint8_t prefixLength,
    uint32_t nextHop,
    const std::string& interfaceName
) {
    if (prefixLength > 32) {
        return;
    }

    routes.push_back({
        network,
        prefixLength,
        nextHop,
        interfaceName,
        false
    });
}

void RouteTable::addDynamicRoute(
    uint32_t network,
    uint8_t prefixLength,
    uint32_t nextHop,
    const std::string& interfaceName
) {
    if (prefixLength > 32) {
        return;
    }

    routes.push_back({
        network,
        prefixLength,
        nextHop,
        interfaceName,
        true
    });
}

void RouteTable::clearDynamicRoutes() {

    routes.erase(
        std::remove_if(
            routes.begin(),
            routes.end(),
            [](const Route& route) {
                return route.dynamic;
            }
        ),
        routes.end()
    );
}

const Route* RouteTable::findRoute(uint32_t destinationIP) const {

    const Route* bestRoute = nullptr;
    int longestPrefix = -1;

    for (const Route& route : routes) {

        uint32_t mask = (route.prefixLength == 0)
            ? 0
            : (0xFFFFFFFFu << (32 - route.prefixLength));

        if ((destinationIP & mask) == (route.network & mask)) {

            if (route.prefixLength > longestPrefix) {
                longestPrefix = route.prefixLength;
                bestRoute = &route;
            }
        }
    }

    return bestRoute;
}