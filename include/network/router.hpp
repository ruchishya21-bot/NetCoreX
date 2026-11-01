#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>

#include "routing/route_table.hpp"
#include "network/network_interface.hpp"
#include "network/access_control_list.hpp"

class Router {

private:
    std::string name;
    RouteTable routingTable;
    InterfaceManager interfaceManager;
    AccessControlList accessControlList;
    bool accessControlEnabled{false};

public:
    Router(const std::string& routerName);

    void addInterface(
        const std::string& name,
        uint32_t ipAddress
    );

    void addRoute(
        uint32_t network,
        uint8_t prefix,
        uint32_t nextHop,
        const std::string& interfaceName
    );

    const std::string& getName() const;

    RouteTable& getRoutingTable();

    InterfaceManager& getInterfaceManager();

    AccessControlList& getAccessControlList();

    void setAccessControlEnabled(bool enabled);

bool isAccessControlEnabled() const;
};

#endif