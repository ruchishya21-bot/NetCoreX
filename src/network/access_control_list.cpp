#include "network/access_control_list.hpp"

bool AccessControlList::matchesPrefix(
    uint32_t ip,
    uint32_t network,
    uint8_t prefix
) const {

    if (prefix > 32) {
        return false;
    }

    if (prefix == 0) {
        return true;
    }

    uint32_t mask = 0xFFFFFFFFu << (32 - prefix);

    return (ip & mask) == (network & mask);
}


void AccessControlList::addRule(const ACLRule& rule) {

    rules.push_back(rule);
}


ACLDecision AccessControlList::evaluate(
    const Packet& packet
) const {

    // Evaluate rules in insertion order.
    for (const ACLRule& rule : rules) {

        bool sourceMatches = matchesPrefix(
            packet.sourceIP,
            rule.sourceNetwork,
            rule.sourcePrefix
        );

        bool destinationMatches = matchesPrefix(
            packet.destinationIP,
            rule.destinationNetwork,
            rule.destinationPrefix
        );

        bool protocolMatches =
            rule.protocol == 0 ||
            rule.protocol == packet.protocol;

        if (sourceMatches &&
            destinationMatches &&
            protocolMatches) {

            return {
                rule.action,
                true,
                rule.action == ACLAction::Permit
                    ? "Packet permitted by ACL"
                    : "Packet denied by ACL"
            };
        }
    }

    // Default-deny policy.
    return {
        ACLAction::Deny,
        false,
        "Packet denied: No matching ACL rule"
    };
}


void AccessControlList::clearRules() {

    rules.clear();
}