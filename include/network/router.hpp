#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <optional>
#include <unordered_map>
#include <memory>

#include "routing/route_table.hpp"
#include "network/network_interface.hpp"
#include "network/access_control_list.hpp"
#include "network/arp_engine.hpp"
#include "connection/connection_tracker.hpp"
#include "nat/nat_engine.hpp"

class Router {

private:
    std::string name;

    RouteTable routingTable;
    InterfaceManager interfaceManager;

    AccessControlList accessControlList;
    ConnectionTracker connectionTracker;

    // One ARP engine per network interface.
    std::unordered_map<std::string, std::unique_ptr<ArpEngine>>
        arpEngines;

    std::optional<NatEngine> natEngine;

    bool accessControlEnabled{false};
    bool statefulFirewallEnabled{false};

public:
    Router(const std::string& routerName);

    void addInterface(
        const std::string& name,
        uint32_t ipAddress
    );

    void addInterface(
        const std::string& name,
        uint32_t ipAddress,
        const MacAddress& macAddress
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

    ConnectionTracker& getConnectionTracker();

    // ARP access.
    ArpEngine* getArpEngine(
        const std::string& interfaceName
    );

    const ArpEngine* getArpEngine(
        const std::string& interfaceName
    ) const;

    std::optional<MacAddress> resolveNeighbor(
        const std::string& interfaceName,
        uint32_t ipAddress
    );

    void setAccessControlEnabled(bool enabled);

    bool isAccessControlEnabled() const;

    void setStatefulFirewallEnabled(bool enabled);

    bool isStatefulFirewallEnabled() const;

    void configureNat(uint32_t publicIP);

    bool isNatEnabled() const;

    NatEngine* getNatEngine();

    const NatEngine* getNatEngine() const;
};

#endif