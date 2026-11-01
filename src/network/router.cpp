#include "network/router.hpp"

Router::Router(const std::string& routerName)
    : name(routerName) {
}

void Router::addInterface(
    const std::string& interfaceName,
    uint32_t ipAddress
) {
    interfaceManager.addInterface(
        interfaceName,
        ipAddress
    );
}

void Router::addRoute(
    uint32_t network,
    uint8_t prefix,
    uint32_t nextHop,
    const std::string& interfaceName
) {
    routingTable.addRoute(
        network,
        prefix,
        nextHop,
        interfaceName
    );
}

const std::string& Router::getName() const {
    return name;
}

RouteTable& Router::getRoutingTable() {
    return routingTable;
}

InterfaceManager& Router::getInterfaceManager() {
    return interfaceManager;
}

AccessControlList& Router::getAccessControlList() {
    return accessControlList;
}

ConnectionTracker& Router::getConnectionTracker() {
    return connectionTracker;
}

void Router::setAccessControlEnabled(bool enabled) {
    accessControlEnabled = enabled;
}

bool Router::isAccessControlEnabled() const {
    return accessControlEnabled;
}

void Router::setStatefulFirewallEnabled(bool enabled) {
    statefulFirewallEnabled = enabled;
}

bool Router::isStatefulFirewallEnabled() const {
    return statefulFirewallEnabled;
}