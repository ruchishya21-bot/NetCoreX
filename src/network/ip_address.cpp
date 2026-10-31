#include "network/ip_address.hpp"

std::string formatIP(uint32_t ip) {

    return std::to_string((ip >> 24) & 255) + "." +
           std::to_string((ip >> 16) & 255) + "." +
           std::to_string((ip >> 8) & 255) + "." +
           std::to_string(ip & 255);
}