#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <cctype>
#include <cstdint>

#include "network/router.hpp"
#include "network/network_simulator.hpp"
#include "network/ip_address.hpp"
#include "routing/control_plane.hpp"

using namespace std;

constexpr uint32_t SOURCE_IP = 0xC0A80102;
constexpr uint32_t DESTINATION_IP = 0xC0A80202;
constexpr uint32_t SUBNET_MASK = 0xFFFFFF00;
constexpr int MAX_TRACE_HOPS = 16;

void heading(const string& title) {
    cout << "\n========== " << title << " ==========\n";
}

Router* findRouter(
    const vector<Router*>& routers,
    const string& name
) {
    string target = name;

    if (target.size() == 2 && target[0] == 'R') {
        target = "Router-" + target.substr(1);
    }

    for (Router* router : routers) {
        if (router->getName() == target) {
            return router;
        }
    }

    return nullptr;
}

void showRouters(const vector<Router*>& routers) {
    heading("REGISTERED ROUTERS");

    for (Router* router : routers) {
        cout << router->getName() << "\n";
    }
}

void showInterfaces(Router& router) {
    heading(router.getName() + " INTERFACES");

    cout << left
         << setw(12) << "Interface"
         << setw(20) << "IP Address"
         << setw(22) << "MAC Address"
         << "Status\n";

    cout << string(65, '-') << "\n";

    for (const auto& interface :
         router.getInterfaceManager().getInterfaces()) {

        cout << left
             << setw(12) << interface.name
             << setw(20) << formatIP(interface.ipAddress)
             << setw(22) << interface.macAddress.toString()
             << (interface.isUp ? "UP" : "DOWN")
             << "\n";
    }
}

void showRoutes(Router& router) {
    heading(router.getName() + " ROUTING TABLE");

    cout << left
         << setw(22) << "Destination"
         << setw(20) << "Next Hop"
         << setw(14) << "Interface"
         << "Type\n";

    cout << string(70, '-') << "\n";

    for (const auto& route :
         router.getRoutingTable().getRoutes()) {

        string destination =
            formatIP(route.network) + "/" +
            to_string(route.prefixLength);

        string nextHop =
            route.nextHop == 0
            ? "Direct"
            : formatIP(route.nextHop);

        cout << left
             << setw(22) << destination
             << setw(20) << nextHop
             << setw(14) << route.interfaceName
             << (route.dynamic ? "Dynamic" : "Static")
             << "\n";
    }
}

void showArp(Router& router) {
    heading(router.getName() + " ARP CACHE");

    bool found = false;

    for (const auto& interface :
         router.getInterfaceManager().getInterfaces()) {

        ArpEngine* engine =
            router.getArpEngine(interface.name);

        if (!engine) {
            continue;
        }

        engine->getArpCache().expireEntries();

        for (const auto& item :
             engine->getArpCache().getEntries()) {

            const ArpEntry& entry = item.second;

            cout << interface.name << " | "
                 << formatIP(entry.ipAddress) << " | "
                 << entry.macAddress.toString() << "\n";

            found = true;
        }
    }

    if (!found) {
        cout << "ARP cache is empty.\n";
    }
}

void showStats(const vector<Router*>& routers) {
    heading("FORWARDING STATISTICS");

    cout << left
         << setw(15) << "Router"
         << setw(15) << "Interface"
         << setw(15) << "Packets"
         << "Bytes\n";

    for (Router* router : routers) {
        for (const auto& interface :
             router->getInterfaceManager().getInterfaces()) {

            cout << left
                 << setw(15) << router->getName()
                 << setw(15) << interface.name
                 << setw(15) << interface.packetsForwarded
                 << interface.bytesForwarded << "\n";
        }
    }
}

ForwardingResult sendPacket(
    NetworkSimulator& simulator,
    Router& source,
    uint32_t sourceIP,
    uint32_t destinationIP
) {
    Packet packet{};

    packet.packetId = 1001;
    packet.sourceIP = sourceIP;
    packet.destinationIP = destinationIP;
    packet.protocol = 1;
    packet.ttl = 64;
    packet.payload = {8, 0, 0, 0};

    cout << "\nSource      : " << formatIP(sourceIP);
    cout << "\nDestination : " << formatIP(destinationIP);
    cout << "\nInitial TTL : " << static_cast<int>(packet.ttl);

    ForwardingResult result =
        simulator.simulate(packet, source);

    cout << "\nFinal TTL   : " << static_cast<int>(packet.ttl);

    cout << "\nResult      : "
         << (result == ForwardingResult::Delivered
             ? "DELIVERED"
             : "FAILED")
         << "\n";

    return result;
}

// Determines whether the destination belongs to a router's
// directly connected /24 interface network.

    bool isDestinationOnLocalLAN(
    Router& router,
    uint32_t destinationIP,
    string& interfaceName
)

{
    for (const auto& interface :
         router.getInterfaceManager().getInterfaces()) {

        if (interface.isUp &&
            (interface.ipAddress & SUBNET_MASK) ==
            (destinationIP & SUBNET_MASK)) {

            interfaceName = interface.name;
            return true;
        }
    }

    return false;
}

// Find the router owning the next-hop interface IP.
Router* findRouterByInterfaceIP(
    const vector<Router*>& routers,
    uint32_t ipAddress
) {
    for (Router* router : routers) {
        for (const auto& interface :
             router->getInterfaceManager().getInterfaces()) {

            if (interface.isUp &&
                interface.ipAddress == ipAddress) {
                return router;
            }
        }
    }

    return nullptr;
}

void tracePacket(
    const vector<Router*>& routers,
    Router& source,
    uint32_t destinationIP
) {
    heading("PACKET ROUTE TRACE");

    cout << "Source router      : " << source.getName() << "\n";
    cout << "Destination IP     : " << formatIP(destinationIP) << "\n\n";

    Router* current = &source;

    unordered_set<string> visited;

    for (int hop = 1; hop <= MAX_TRACE_HOPS; ++hop) {

        if (!visited.insert(current->getName()).second) {
            cout << "Routing loop detected at "
                 << current->getName() << ".\n";
            return;
        }

        string destinationInterface;

        if (isDestinationOnLocalLAN(
                *current,
                destinationIP,
                destinationInterface)) {

            cout << hop << ". "
                 << current->getName()
                 << " -> Destination LAN"
                 << " | Interface: " << destinationInterface
                 << " | " << formatIP(destinationIP)
                 << "\n";

            cout << "\nTrace complete.\n";
            return;
        }

        const Route* route =
            current->getRoutingTable().findRoute(destinationIP);

        if (!route) {
            cout << hop << ". "
                 << current->getName()
                 << " -> No route to destination.\n";
            cout << "\nTrace failed.\n";
            return;
        }

        const auto* outgoingInterface =
            current->getInterfaceManager().findInterface(
                route->interfaceName
            );

        if (!outgoingInterface || !outgoingInterface->isUp) {
            cout << hop << ". "
                 << current->getName()
                 << " -> Outgoing interface unavailable: "
                 << route->interfaceName << "\n";
            return;
        }

        if (route->nextHop == 0) {
            cout << hop << ". "
                 << current->getName()
                 << " -> Direct route through "
                 << route->interfaceName << "\n";
            cout << "\nTrace stopped: destination is not a known router.\n";
            return;
        }

        cout << hop << ". "
             << current->getName()
             << " -> " << formatIP(route->nextHop)
             << " | Interface: " << route->interfaceName
             << "\n";

        Router* nextRouter =
            findRouterByInterfaceIP(routers, route->nextHop);

        if (!nextRouter) {
            cout << "Next-hop router is not registered.\n";
            return;
        }

        current = nextRouter;
    }

    cout << "\nMaximum trace hop limit reached.\n";
}

void printHelp() {
    heading("AVAILABLE COMMANDS");

    cout << "show routers          - List routers\n";
    cout << "show interfaces R1    - Show interfaces\n";
    cout << "show routes R1        - Show routing table\n";
    cout << "show arp R1           - Show ARP cache\n";
    cout << "show stats            - Show forwarding statistics\n";
    cout << "ping R1 R4            - Simulate packet forwarding\n";
    cout << "trace R1 R4           - Trace routing path\n";
    cout << "failover              - Remove primary routing link\n";
    cout << "help                  - Show commands\n";
    cout << "exit                  - Exit console\n";
}

int main() {

    cout << "\n";
    cout << "============================================\n";
    cout << "                 NetCoreX\n";
    cout << "        NETWORK INTELLIGENCE CLI\n";
    cout << "============================================\n";

    Router r1("Router-1");
    Router r2("Router-2");
    Router r3("Router-3");
    Router r4("Router-4");

    r1.addInterface("eth12", 0x0A000001);
    r1.addInterface("eth13", 0x0A000002);
    r1.addInterface("ethLAN", 0xC0A80101);

    r2.addInterface("eth21", 0x0A000011);
    r2.addInterface("eth24", 0x0A000012);

    r3.addInterface("eth31", 0x0A000021);
    r3.addInterface("eth34", 0x0A000022);

    r4.addInterface("eth42", 0x0A000031);
    r4.addInterface("eth43", 0x0A000032);
    r4.addInterface("ethLAN", 0xC0A80201);

    vector<Router*> routers = {&r1, &r2, &r3, &r4};

    ControlPlane controlPlane;

    for (Router* router : routers) {
        controlPlane.registerRouter(*router);
    }

    controlPlane.addLink(
        "Router-1", "eth12",
        "Router-2", "eth21", 2
    );

    controlPlane.addLink(
        "Router-2", "eth24",
        "Router-4", "eth42", 2
    );

    controlPlane.addLink(
        "Router-1", "eth13",
        "Router-3", "eth31", 5
    );

    controlPlane.addLink(
        "Router-3", "eth34",
        "Router-4", "eth43", 5
    );

    controlPlane.advertiseNetwork(
        "Router-4",
        0xC0A80200,
        24,
        "ethLAN"
    );

    controlPlane.installRoutes();

    NetworkSimulator simulator;

    for (Router* router : routers) {
        simulator.addRouter(*router);
    }

    simulator.connectInterfaces(r1, "eth12", r2, "eth21");
    simulator.connectInterfaces(r1, "eth13", r3, "eth31");
    simulator.connectInterfaces(r2, "eth24", r4, "eth42");
    simulator.connectInterfaces(r3, "eth34", r4, "eth43");

    cout << "\nNetwork initialized successfully.";
    cout << "\nRouters: 4";
    cout << "\nEthernet links: " << simulator.getLinkCount();
    cout << "\nType 'help' to see available commands.\n";

    string line;

    while (true) {

        cout << "\nNetCoreX> ";

        if (!getline(cin, line)) {
            break;
        }

        // Normalize input, including Windows-style carriage returns.
        line.erase(
            remove(line.begin(), line.end(), '\r'),
            line.end()
        );

        stringstream input(line);

        string command;
        string argument1;
        string argument2;

        input >> command >> argument1 >> argument2;

        transform(
            command.begin(),
            command.end(),
            command.begin(),
            [](unsigned char c) {
                return static_cast<char>(tolower(c));
            }
        );

        if (command == "exit" || command == "quit") {
            cout << "Closing NetCoreX.\n";
            break;
        }

        else if (command == "help") {
            printHelp();
        }

        else if (command == "show" && argument1 == "routers") {
            showRouters(routers);
        }

        else if (command == "show" && argument1 == "stats") {
            showStats(routers);
        }

        else if (command == "show" && argument1 == "interfaces") {
            Router* router = findRouter(routers, argument2);

            if (router) {
                showInterfaces(*router);
            } else {
                cout << "Router not found.\n";
            }
        }

        else if (command == "show" && argument1 == "routes") {
            Router* router = findRouter(routers, argument2);

            if (router) {
                showRoutes(*router);
            } else {
                cout << "Router not found.\n";
            }
        }

        else if (command == "show" && argument1 == "arp") {
            Router* router = findRouter(routers, argument2);

            if (router) {
                showArp(*router);
            } else {
                cout << "Router not found.\n";
            }
        }

        else if (command == "ping") {
            Router* source = findRouter(routers, argument1);
            Router* destination = findRouter(routers, argument2);

            if (!source || !destination) {
                cout << "Invalid router name.\n";
                continue;
            }

            sendPacket(
                simulator,
                *source,
                SOURCE_IP,
                DESTINATION_IP
            );
        }

        else if (command == "trace") {
            Router* source = findRouter(routers, argument1);
            Router* destination = findRouter(routers, argument2);

            if (!source || !destination) {
                cout << "Invalid router name.\n";
                continue;
            }

            tracePacket(
                routers,
                *source,
                DESTINATION_IP
            );
        }

        else if (command == "failover") {
            cout << "Removing primary control-plane link...\n";

            controlPlane.removeLink("Router-2", "Router-4");
            controlPlane.installRoutes();

            cout << "Routes recalculated.\n";
            cout << "Try: show routes R1\n";
            cout << "Then: trace R1 R4\n";
        }

        else if (command.empty()) {
            continue;
        }

        else {
            cout << "Unknown command. Type 'help'.\n";
        }
    }

    return 0;
}