#include "routing/link_state_routing.hpp"

#include <iostream>
#include <cassert>

int main() {


LinkStateRouting routing;

routing.addLink("Router-1", "Router-2", 4);
routing.addLink("Router-1", "Router-3", 2);
routing.addLink("Router-3", "Router-2", 1);
routing.addLink("Router-2", "Router-4", 5);
routing.addLink("Router-3", "Router-4", 8);

// Test 1: Shortest path
PathResult result =
    routing.findShortestPath("Router-1", "Router-4");

assert(result.reachable);
assert(result.totalCost == 8);
assert(result.nextHop == "Router-3");

std::cout << "Test 1 passed: Shortest path.\n";

// Test 2: Unreachable destination
PathResult unreachable =
    routing.findShortestPath("Router-1", "Router-5");

assert(!unreachable.reachable);

std::cout << "Test 2 passed: Unreachable destination.\n";

// Test 3: Same source and destination
PathResult same =
    routing.findShortestPath("Router-1", "Router-1");

assert(same.reachable);
assert(same.totalCost == 0);

std::cout << "Test 3 passed: Same router.\n";

// Test 4: Remove a link used by the shortest path
routing.removeLink("Router-3", "Router-2");

PathResult alternative =
    routing.findShortestPath("Router-1", "Router-4");

assert(alternative.reachable);
assert(alternative.totalCost == 9);
assert(alternative.nextHop == "Router-2");

std::cout << "Test 4 passed: Alternative route calculated.\n";

// Test 5: Remove another link
routing.removeLink("Router-2", "Router-4");

PathResult remaining =
    routing.findShortestPath("Router-1", "Router-4");

assert(remaining.reachable);
assert(remaining.totalCost == 10);
assert(remaining.nextHop == "Router-3");

std::cout << "Test 5 passed: Second alternative route calculated.\n";

// Test 6: Disconnect the final path
routing.removeLink("Router-3", "Router-4");

PathResult failed =
    routing.findShortestPath("Router-1", "Router-4");

assert(!failed.reachable);

std::cout << "Test 6 passed: Network failure detected.\n";

std::cout << "\nAll routing tests passed!\n";

return 0;


}
