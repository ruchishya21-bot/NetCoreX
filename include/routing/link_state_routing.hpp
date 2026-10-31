#ifndef LINK_STATE_ROUTING_HPP
#define LINK_STATE_ROUTING_HPP

#include <string>
#include <unordered_map>
#include <vector>

struct PathResult {
    bool reachable;
    int totalCost;
    std::string nextHop;
};

class LinkStateRouting {

private:
    std::unordered_map<
        std::string,
        std::vector<std::pair<std::string, int>>
    > networkGraph;

public:
    void addLink(
        const std::string& routerA,
        const std::string& routerB,
        int cost
    );


     void removeLink(
        const std::string& routerA,
        const std::string& routerB
    );


    PathResult findShortestPath(
        const std::string& source,
        const std::string& destination
    );
};

#endif