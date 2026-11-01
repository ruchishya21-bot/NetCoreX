#ifndef ACCESS_CONTROL_LIST_HPP
#define ACCESS_CONTROL_LIST_HPP

#include <cstdint>
#include <vector>
#include <string>

#include "packet/packet.hpp"

enum class ACLAction {
    Permit,
    Deny
};

struct ACLRule {
    uint32_t sourceNetwork;
    uint8_t sourcePrefix;

    uint32_t destinationNetwork;
    uint8_t destinationPrefix;

    uint8_t protocol;  // 0 = Any, 6 = TCP, 17 = UDP, 1 = ICMP

    ACLAction action;
};

struct ACLDecision {
    ACLAction action;
    bool matched;
    std::string message;
};

class AccessControlList {
private:
    std::vector<ACLRule> rules;

    bool matchesPrefix(
        uint32_t ip,
        uint32_t network,
        uint8_t prefix
    ) const;

public:
    void addRule(const ACLRule& rule);

    ACLDecision evaluate(const Packet& packet) const;

    void clearRules();
};

#endif