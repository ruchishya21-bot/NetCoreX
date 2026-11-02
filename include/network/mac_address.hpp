#ifndef MAC_ADDRESS_HPP
#define MAC_ADDRESS_HPP

#include <array>
#include <cstdint>
#include <string>
#include <optional>

class MacAddress {

private:
    std::array<uint8_t, 6> address{};

public:
    MacAddress() = default;

    explicit MacAddress(
        const std::array<uint8_t, 6>& bytes
    );

    static std::optional<MacAddress> parse(
        const std::string& value
    );

    std::string toString() const;

    const std::array<uint8_t, 6>& getBytes() const;

    bool operator==(const MacAddress& other) const;

    bool operator!=(const MacAddress& other) const;
};

#endif