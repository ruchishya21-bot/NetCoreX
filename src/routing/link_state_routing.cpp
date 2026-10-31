#include "routing/link_state_routing.hpp"

#include <queue>
#include <unordered_map>
#include <limits>
#include <functional>
#include <algorithm>

void LinkStateRouting::addLink(
    const std::string& routerA,
    const std::string& routerB,
    int cost
) {
    if (cost <= 0 || routerA == routerB) {
        return;
    }

    removeLink(routerA, routerB);

    networkGraph[routerA].push_back({routerB, cost});
    networkGraph[routerB].push_back({routerA, cost});
}

void LinkStateRouting::removeLink(
    const std::string& routerA,
    const std::string& routerB
) {
    auto removeNeighbor = [](auto& neighbors, const std::string& target) {
        neighbors.erase(
            std::remove_if(
                neighbors.begin(),
                neighbors.end(),
                [&](const auto& link) {
                    return link.first == target;
                }
            ),
            neighbors.end()
        );
    };

    if (networkGraph.count(routerA)) {
        removeNeighbor(networkGraph[routerA], routerB);
    }

    if (networkGraph.count(routerB)) {
        removeNeighbor(networkGraph[routerB], routerA);
    }
}

PathResult LinkStateRouting::findShortestPath(
    const std::string& source,
    const std::string& destination
) {
    if (source == destination) {
        return {true, 0, source};
    }

    const int INF = std::numeric_limits<int>::max();

    std::unordered_map<std::string, int> distance;
    std::unordered_map<std::string, std::string> firstHop;

    using Node = std::pair<int, std::string>;

    std::priority_queue<
        Node,
        std::vector<Node>,
        std::greater<Node>
    > pq;

    distance[source] = 0;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [currentCost, currentRouter] = pq.top();
        pq.pop();

        if (currentCost > distance[currentRouter]) {
            continue;
        }

        if (currentRouter == destination) {
            break;
        }

        if (!networkGraph.count(currentRouter)) {
            continue;
        }

        for (const auto& link : networkGraph[currentRouter]) {
            const std::string& neighbor = link.first;
            int linkCost = link.second;

            if (currentCost > INF - linkCost) {
                continue;
            }

            int newCost = currentCost + linkCost;

            if (!distance.count(neighbor) ||
                newCost < distance[neighbor]) {

                distance[neighbor] = newCost;

                firstHop[neighbor] =
                    (currentRouter == source)
                    ? neighbor
                    : firstHop[currentRouter];

                pq.push({newCost, neighbor});
            }
        }
    }

    if (!distance.count(destination)) {
        return {false, -1, ""};
    }

    return {
        true,
        distance[destination],
        firstHop[destination]
    };
}