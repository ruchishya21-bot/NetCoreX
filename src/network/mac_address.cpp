#include "network/mac_address.hpp"

#include <sstream>
#include <iomanip>
#include <stdexcept>

MacAddress::MacAddress(
    const std::array<uint8_t, 6>& bytes
)
    : address(bytes) {
}

std::optional<MacAddress> MacAddress::parse(
    const std::string& value
) {

    std::array<uint8_t, 6> bytes{};

    std::istringstream stream(value);

    std::string segment;

    for (int i = 0; i < 6; i++) {

        if (!std::getline(stream, segment, ':')) {
            return std::nullopt;
        }

        if (segment.length() != 2) {
            return std::nullopt;
        }

        try {

            size_t processed = 0;

            unsigned long number =
                std::stoul(segment, &processed, 16);

            if (processed != 2 || number > 255) {
                return std::nullopt;
            }

            bytes[i] = static_cast<uint8_t>(number);

        }
        catch (const std::exception&) {

            return std::nullopt;
        }
    }

    if (std::getline(stream, segment, ':')) {
        return std::nullopt;
    }

    return MacAddress(bytes);
}

std::string MacAddress::toString() const {

    std::ostringstream stream;

    stream << std::hex
           << std::setfill('0')
           << std::nouppercase;

    for (size_t i = 0; i < address.size(); i++) {

        if (i > 0) {
            stream << ":";
        }

        stream << std::setw(2)
               << static_cast<int>(address[i]);
    }

    return stream.str();
}

const std::array<uint8_t, 6>&
MacAddress::getBytes() const {

    return address;
}

bool MacAddress::operator==(
    const MacAddress& other
) const {

    return address == other.address;
}

bool MacAddress::operator!=(
    const MacAddress& other
) const {

    return !(*this == other);
}