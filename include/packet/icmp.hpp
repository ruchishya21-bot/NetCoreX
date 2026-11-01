#ifndef ICMP_HPP
#define ICMP_HPP

#include <cstdint>
#include <string>

enum class IcmpType : uint8_t {
    DestinationUnreachable = 3,
    TimeExceeded = 11
};

enum class IcmpCode : uint8_t {
    NetworkUnreachable = 0,
    HostUnreachable = 1,
    TtlExpired = 0
};

struct IcmpError {
    IcmpType type;
    IcmpCode code;

    std::string reportingRouter;

    uint32_t originalSourceIP;
    uint32_t originalDestinationIP;
    uint32_t originalPacketId;

    std::string message;
};

#endif