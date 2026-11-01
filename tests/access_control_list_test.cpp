#include <iostream>
#include <cassert>

#include "network/access_control_list.hpp"

void testPermitRule() {
    AccessControlList acl;
    Packet packet{};

    packet.sourceIP = 0xC0A80110;
    packet.destinationIP = 0xC0A80220;
    packet.protocol = 6; // TCP

    acl.addRule({
        0xC0A80100, 24,
        0xC0A80200, 24,
        6,
        ACLAction::Permit
    });

    ACLDecision decision = acl.evaluate(packet);

    assert(decision.matched);
    assert(decision.action == ACLAction::Permit);

    std::cout << "PASS: TCP packet permitted\n";
}

void testDenyRule() {
    AccessControlList acl;
    Packet packet{};

    packet.sourceIP = 0xC0A80110;
    packet.destinationIP = 0xC0A80220;
    packet.protocol = 6;

    acl.addRule({
        0xC0A80100, 24,
        0xC0A80200, 24,
        6,
        ACLAction::Deny
    });

    ACLDecision decision = acl.evaluate(packet);

    assert(decision.matched);
    assert(decision.action == ACLAction::Deny);

    std::cout << "PASS: TCP packet denied\n";
}

void testRuleOrdering() {
    AccessControlList acl;
    Packet packet{};

    packet.sourceIP = 0xC0A80110;
    packet.destinationIP = 0xC0A80220;
    packet.protocol = 6;

    // First rule denies the specific source.
    acl.addRule({
        0xC0A80110, 32,
        0x00000000, 0,
        0,
        ACLAction::Deny
    });

    // Second rule permits the entire source network.
    acl.addRule({
        0xC0A80100, 24,
        0xC0A80200, 24,
        6,
        ACLAction::Permit
    });

    ACLDecision decision = acl.evaluate(packet);

    assert(decision.matched);
    assert(decision.action == ACLAction::Deny);

    std::cout << "PASS: ACL rule ordering\n";
}

void testDefaultDeny() {
    AccessControlList acl;
    Packet packet{};

    packet.sourceIP = 0x0A000001;
    packet.destinationIP = 0xC0A80220;
    packet.protocol = 17; // UDP

    ACLDecision decision = acl.evaluate(packet);

    assert(!decision.matched);
    assert(decision.action == ACLAction::Deny);

    std::cout << "PASS: Default-deny policy\n";
}

void testProtocolMismatch() {
    AccessControlList acl;
    Packet packet{};

    packet.sourceIP = 0xC0A80110;
    packet.destinationIP = 0xC0A80220;
    packet.protocol = 17; // UDP

    acl.addRule({
        0xC0A80100, 24,
        0xC0A80200, 24,
        6,
        ACLAction::Permit
    });

    ACLDecision decision = acl.evaluate(packet);

    assert(!decision.matched);
    assert(decision.action == ACLAction::Deny);

    std::cout << "PASS: Protocol filtering\n";
}

int main() {
    testPermitRule();
    testDenyRule();
    testRuleOrdering();
    testDefaultDeny();
    testProtocolMismatch();

    std::cout << "\nAll ACL tests passed!\n";

    return 0;
}