#include <cassert>
#include <iostream>

#include "network/arp_engine.hpp"

void testArpRequestCreation() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac.has_value());

    ArpEngine engine(0xC0A8010A, *mac);

    ArpMessage request =
        engine.createRequest(0xC0A80114);

    assert(request.operation == ArpOperation::Request);
    assert(request.senderIP == 0xC0A8010A);
    assert(request.targetIP == 0xC0A80114);

    assert(request.senderMAC == *mac);

    std::cout << "PASS: ARP request creation\n";
}

void testArpRequestAndReply() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA.has_value());
    assert(macB.has_value());

    ArpEngine deviceA(0xC0A8010A, *macA);
    ArpEngine deviceB(0xC0A80114, *macB);

    ArpMessage request =
        deviceA.createRequest(0xC0A80114);

    auto reply =
        deviceB.processRequest(request);

    assert(reply.has_value());

    assert(reply->operation == ArpOperation::Reply);
    assert(reply->senderIP == 0xC0A80114);
    assert(reply->targetIP == 0xC0A8010A);

    assert(reply->senderMAC == *macB);
    assert(reply->targetMAC == *macA);

    std::cout << "PASS: ARP request and reply\n";
}

void testReplyLearning() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA.has_value());
    assert(macB.has_value());

    ArpEngine deviceA(0xC0A8010A, *macA);
    ArpEngine deviceB(0xC0A80114, *macB);

    ArpMessage request =
        deviceA.createRequest(0xC0A80114);

    auto reply =
        deviceB.processRequest(request);

    assert(reply.has_value());

    bool accepted =
        deviceA.processReply(*reply);

    assert(accepted);

    auto resolved =
        deviceA.resolve(0xC0A80114);

    assert(resolved.has_value());
    assert(*resolved == *macB);

    std::cout << "PASS: ARP reply learning\n";
}

void testUnknownTargetDoesNotReply() {

    auto mac = MacAddress::parse("00:11:22:33:44:55");

    assert(mac.has_value());

    ArpEngine engine(0xC0A8010A, *mac);

    ArpMessage request =
        engine.createRequest(0xC0A80199);

    auto reply =
        engine.processRequest(request);

    assert(!reply.has_value());

    std::cout << "PASS: Unknown target does not reply\n";
}

void testRequestLearnsSender() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA.has_value());
    assert(macB.has_value());

    ArpEngine deviceA(0xC0A8010A, *macA);
    ArpEngine deviceB(0xC0A80114, *macB);

    ArpMessage request =
        deviceA.createRequest(0xC0A80199);

    deviceB.processRequest(request);

    auto result =
        deviceB.resolve(0xC0A8010A);

    assert(result.has_value());
    assert(*result == *macA);

    std::cout << "PASS: Request sender learning\n";
}

void testInvalidReplyRejected() {

    auto macA = MacAddress::parse("00:11:22:33:44:55");
    auto macB = MacAddress::parse("AA:BB:CC:DD:EE:FF");

    assert(macA.has_value());
    assert(macB.has_value());

    ArpEngine deviceA(0xC0A8010A, *macA);

    ArpMessage reply;

    reply.operation = ArpOperation::Reply;
    reply.senderIP = 0xC0A80114;
    reply.senderMAC = *macB;

    // Reply is addressed to another IP.
    reply.targetIP = 0xC0A80199;
    reply.targetMAC = *macA;

    assert(!deviceA.processReply(reply));

    std::cout << "PASS: Invalid ARP reply rejection\n";
}

int main() {

    testArpRequestCreation();
    testArpRequestAndReply();
    testReplyLearning();
    testUnknownTargetDoesNotReply();
    testRequestLearnsSender();
    testInvalidReplyRejected();

    std::cout << "\nAll ARP engine tests passed.\n";

    return 0;
}