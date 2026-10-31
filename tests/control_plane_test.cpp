#include "routing/control_plane.hpp"

#include <cassert>
#include <iostream>

int main() {

    Router r1("Router-1");
    Router r2("Router-2");
    Router r3("Router-3");

    // Configure Router-1 interfaces
    r1.addInterface("eth1", 0x0A000001);
    r1.addInterface("eth2", 0x0A000005);

    // Configure Router-2 interfaces
    r2.addInterface("eth1", 0x0A000002);
    r2.addInterface("eth2", 0x0A000009);

    // Configure Router-3 interfaces
    r3.addInterface("eth1", 0x0A00000A);
    r3.addInterface("eth2", 0x0A000006);
    r3.addInterface("eth0", 0xC0A80101);

    ControlPlane controlPlane;

    controlPlane.registerRouter(r1);
    controlPlane.registerRouter(r2);
    controlPlane.registerRouter(r3);

    // Create network links
    assert(controlPlane.addLink(
        "Router-1", "eth1", "Router-2", "eth1", 2
    ));

    assert(controlPlane.addLink(
        "Router-2", "eth2", "Router-3", "eth1", 2
    ));

    assert(controlPlane.addLink(
        "Router-1", "eth2", "Router-3", "eth2", 10
    ));

    // Advertise destination network
    assert(controlPlane.advertiseNetwork(
        "Router-3",
        0xC0A80100,
        24,
        "eth0"
    ));

    // Install routes
    controlPlane.installRoutes();

    // Verify shortest path uses Router-2
    const Route* route =
        r1.getRoutingTable().findRoute(0xC0A80102);

    assert(route != nullptr);
    assert(route->dynamic);
    assert(route->nextHop == 0x0A000002);
    assert(route->interfaceName == "eth1");

    std::cout << "Test 1 passed: Automatic route installation.\n";

    // Simulate failure of Router-2 to Router-3 link
    controlPlane.removeLink("Router-2", "Router-3");

    // Recalculate and reinstall routes
    controlPlane.installRoutes();

    route = r1.getRoutingTable().findRoute(0xC0A80102);

    // Verify alternative path uses Router-3 directly
    assert(route != nullptr);
    assert(route->dynamic);
    assert(route->nextHop == 0x0A000006);
    assert(route->interfaceName == "eth2");

    std::cout << "Test 2 passed: Automatic alternative route.\n";

    std::cout << "\nAll Control Plane tests passed!\n";

    return 0;
}