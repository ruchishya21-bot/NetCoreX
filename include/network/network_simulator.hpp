#ifndef NETWORK_SIMULATOR_HPP
#define NETWORK_SIMULATOR_HPP

#include <vector>

#include "network/router.hpp"
#include "packet/packet.hpp"

class NetworkSimulator {

private:
    std::vector<Router*> routers;

public:
    void addRouter(Router& router);

    void simulate(Packet& packet, Router& sourceRouter);
};

#endif