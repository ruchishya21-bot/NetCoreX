#include "routing/control_plane.hpp"

#include <algorithm>
#include <limits>

void ControlPlane::registerRouter(Router& router) {

    routers[router.getName()] = &router;
}

bool ControlPlane::addLink(
    const std::string& routerA,
    const std::string& interfaceA,
    const std::string& routerB,
    const std::string& interfaceB,
    int cost
) {

    if (cost <= 0 || routerA == routerB) {
        return false;
    }

    if (!routers.count(routerA) || !routers.count(routerB)) {
        return false;
    }

    auto* intA =
        routers[routerA]->getInterfaceManager().findInterface(interfaceA);

    auto* intB =
        routers[routerB]->getInterfaceManager().findInterface(interfaceB);

    if (!intA || !intB || !intA->isUp || !intB->isUp) {
        return false;
    }

    removeLink(routerA, routerB);

    routing.addLink(routerA, routerB, cost);

    links[{routerA, routerB}] = {
        interfaceA,
        interfaceB
    };

    links[{routerB, routerA}] = {
        interfaceB,
        interfaceA
    };

    return true;
}

void ControlPlane::removeLink(
    const std::string& routerA,
    const std::string& routerB
) {

    routing.removeLink(routerA, routerB);

    links.erase({routerA, routerB});
    links.erase({routerB, routerA});
}

bool ControlPlane::advertiseNetwork(
    const std::string& routerName,
    uint32_t network,
    uint8_t prefix,
    const std::string& interfaceName
) {

    if (prefix > 32 || !routers.count(routerName)) {
        return false;
    }

    auto* interface =
        routers[routerName]->getInterfaceManager().findInterface(interfaceName);

    if (!interface || !interface->isUp) {
        return false;
    }

    advertisements.push_back({
        network,
        prefix,
        routerName,
        interfaceName
    });

    return true;
}

void ControlPlane::installRoutes() {

    // Remove previously installed dynamic routes.
    for (auto& entry : routers) {
        entry.second->getRoutingTable().clearDynamicRoutes();
    }

    using RouteKey =
        std::tuple<std::string, uint32_t, uint8_t>;

    std::map<RouteKey, RouteCandidate> bestRoutes;

    // Calculate routes to every advertised network.
    for (auto& sourceEntry : routers) {

        const std::string& source = sourceEntry.first;

        for (const auto& advertisement : advertisements) {

            auto advertiser = routers.find(advertisement.routerName);

            if (advertiser == routers.end()) {
                continue;
            }

            auto* destinationInterface =
                advertiser->second->getInterfaceManager()
                    .findInterface(advertisement.interfaceName);

            if (!destinationInterface || !destinationInterface->isUp) {
                continue;
            }

            PathResult path = routing.findShortestPath(
                source,
                advertisement.routerName
            );

            if (!path.reachable) {
                continue;
            }

            uint32_t nextHop = 0;
            std::string outputInterface;

            // Destination network is directly connected.
            if (source == advertisement.routerName) {

                outputInterface = advertisement.interfaceName;

            } else {

                auto link = links.find({
                    source,
                    path.nextHop
                });

                if (link == links.end()) {
                    continue;
                }

                auto* localInterface =
                    sourceEntry.second->getInterfaceManager()
                        .findInterface(link->second.interfaceName);

                auto* remoteInterface =
                    routers[path.nextHop]->getInterfaceManager()
                        .findInterface(link->second.neighborInterface);

                if (!localInterface || !remoteInterface ||
                    !localInterface->isUp || !remoteInterface->isUp) {
                    continue;
                }

                nextHop = remoteInterface->ipAddress;
                outputInterface = localInterface->name;
            }

            uint32_t mask = advertisement.prefix == 0
                ? 0
                : (0xFFFFFFFFu << (32 - advertisement.prefix));

            uint32_t network = advertisement.network & mask;

            RouteKey key = {
                source,
                network,
                advertisement.prefix
            };

            auto existing = bestRoutes.find(key);

            if (existing == bestRoutes.end() ||
                path.totalCost < existing->second.cost) {

                bestRoutes[key] = {
                    path.totalCost,
                    nextHop,
                    outputInterface
                };
            }
        }
    }

    // Install calculated routes into each router.
    for (const auto& entry : bestRoutes) {

        const auto& key = entry.first;
        const auto& candidate = entry.second;

        const std::string& routerName = std::get<0>(key);
        uint32_t network = std::get<1>(key);
        uint8_t prefix = std::get<2>(key);

        routers[routerName]->getRoutingTable().addDynamicRoute(
            network,
            prefix,
            candidate.nextHop,
            candidate.interfaceName
        );
    }
}