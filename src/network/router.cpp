#include "network/router.hpp"

Router::Router(const std::string& routerName)
    : name(routerName) {
}

void Router::addInterface(
    const std::string& interfaceName,
    uint32_t ipAddress
) {
    interfaceManager.addInterface(interfaceName, ipAddress);

    auto* interface =
        interfaceManager.findInterface(interfaceName);

    if (interface != nullptr) {
        arpEngines[interfaceName] =
            std::make_unique<ArpEngine>(
                ipAddress,
                interface->macAddress
            );
    }
}

void Router::addInterface(
    const std::string& interfaceName,
    uint32_t ipAddress,
    const MacAddress& macAddress
) {
    interfaceManager.addInterface(
        interfaceName,
        ipAddress,
        macAddress
    );

    auto* interface =
        interfaceManager.findInterface(interfaceName);

    if (interface != nullptr) {
        arpEngines[interfaceName] =
            std::make_unique<ArpEngine>(
                ipAddress,
                interface->macAddress
            );
    }
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

ArpEngine* Router::getArpEngine(
    const std::string& interfaceName
) {
    auto it = arpEngines.find(interfaceName);

    if (it == arpEngines.end()) {
        return nullptr;
    }

    return it->second.get();
}

const ArpEngine* Router::getArpEngine(
    const std::string& interfaceName
) const {
    auto it = arpEngines.find(interfaceName);

    if (it == arpEngines.end()) {
        return nullptr;
    }

    return it->second.get();
}

std::optional<MacAddress> Router::resolveNeighbor(
    const std::string& interfaceName,
    uint32_t ipAddress
) {
    ArpEngine* engine = getArpEngine(interfaceName);

    if (engine == nullptr) {
        return std::nullopt;
    }

    return engine->resolve(ipAddress);
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

void Router::configureNat(uint32_t publicIP) {
    natEngine.emplace(publicIP);
}

bool Router::isNatEnabled() const {
    return natEngine.has_value();
}

NatEngine* Router::getNatEngine() {
    return natEngine ? &(*natEngine) : nullptr;
}

const NatEngine* Router::getNatEngine() const {
    return natEngine ? &(*natEngine) : nullptr;
}