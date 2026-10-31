#ifndef CONTROL_PLANE_HPP
#define CONTROL_PLANE_HPP

#include "network/router.hpp"
#include "routing/link_state_routing.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <tuple>

class ControlPlane {

private:

    struct LinkInfo {
        std::string interfaceName;
        std::string neighborInterface;
    };

    struct NetworkAdvertisement {
        uint32_t network;
        uint8_t prefix;
        std::string routerName;
        std::string interfaceName;
    };

    struct RouteCandidate {
        int cost;
        uint32_t nextHop;
        std::string interfaceName;
    };

    LinkStateRouting routing;

    std::unordered_map<std::string, Router*> routers;

    std::map<
        std::pair<std::string, std::string>,
        LinkInfo
    > links;

    std::vector<NetworkAdvertisement> advertisements;

public:

    void registerRouter(Router& router);

    bool addLink(
        const std::string& routerA,
        const std::string& interfaceA,
        const std::string& routerB,
        const std::string& interfaceB,
        int cost
    );

    void removeLink(
        const std::string& routerA,
        const std::string& routerB
    );

    bool advertiseNetwork(
        const std::string& routerName,
        uint32_t network,
        uint8_t prefix,
        const std::string& interfaceName
    );

    void installRoutes();
};

#endif