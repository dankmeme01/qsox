#pragma once

#include <array>
#include <compare>
#include <stdint.h>
#include <stddef.h>
#include "Error.hpp"

struct in_addr;

namespace qsox {

QSOX_MAKE_ERROR_STRUCT(Ipv4ParseError,
    MissingOctets,
    TrailingData,
    InvalidOctet
);

class Ipv4Address {
public:
    // Represents 127.0.0.1
    static Ipv4Address LOCALHOST;

    // Represents 0.0.0.0
    static Ipv4Address UNSPECIFIED;

    // Represents 255.255.255.255
    static Ipv4Address BROADCAST;

    constexpr inline Ipv4Address() : Ipv4Address(0, 0, 0, 0) {}
    constexpr inline Ipv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : m_octets{a, b, c, d} {}
    constexpr inline Ipv4Address(const std::array<uint8_t, 4>& octets) : m_octets(octets) {}

    constexpr inline Ipv4Address(const Ipv4Address& other) = default;
    constexpr inline Ipv4Address& operator=(const Ipv4Address& other) = default;

    constexpr inline static Ipv4Address fromBits(uint32_t bits) {
        return Ipv4Address(
            static_cast<uint8_t>((bits >> 24) & 0xFF),
            static_cast<uint8_t>((bits >> 16) & 0xFF),
            static_cast<uint8_t>((bits >> 8) & 0xFF),
            static_cast<uint8_t>(bits & 0xFF)
        );
    }

    constexpr inline uint32_t toBits() const {
        return (static_cast<uint32_t>(m_octets[0]) << 24) |
               (static_cast<uint32_t>(m_octets[1]) << 16) |
               (static_cast<uint32_t>(m_octets[2]) << 8) |
               static_cast<uint32_t>(m_octets[3]);
    }

    constexpr inline uint8_t operator[](size_t index) const {
        return m_octets[index];
    }

    constexpr inline uint8_t& operator[](size_t index) {
        return m_octets[index];
    }

    constexpr inline bool operator==(const Ipv4Address& other) const {
        return m_octets == other.m_octets;
    }

    constexpr inline bool operator!=(const Ipv4Address& other) const {
        return !(*this == other);
    }

    constexpr inline std::strong_ordering operator<=>(const Ipv4Address& rhs) {
        if (m_octets == rhs.m_octets) {
            return std::strong_ordering::equal;
        }

        return m_octets < rhs.m_octets ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    constexpr inline const std::array<uint8_t, 4>& octets() const {
        return m_octets;
    }

    constexpr inline std::array<uint8_t, 4>& octets() {
        return m_octets;
    }

    static Result<Ipv4Address, Ipv4ParseError> parse(std::string_view str);
    std::string toString() const;
    void toInAddr(in_addr& addr) const;
    static Ipv4Address fromInAddr(const in_addr& addr);

    // Returns whether the address is unspecified (0.0.0.0)
    constexpr inline bool isUnspecified() const {
        return m_octets == std::array<uint8_t, 4>{0, 0, 0, 0};
    }

    // Returns whether the address is a loopback address (127.0.0.0/8)
    constexpr inline bool isLocalhost() const {
        return m_octets[0] == 127;
    }

    // Returns whether the address is an address in a private range (as per RFC 1918)
    constexpr bool isPrivate() const {
        return (m_octets[0] == 10) ||
                (m_octets[0] == 172 && m_octets[1] >= 16 && m_octets[1] <= 31) ||
                (m_octets[0] == 192 && m_octets[1] == 168);
    }

    // Returns whether the address is a broadcast address
    constexpr inline bool isBroadcast() const {
        return m_octets == BROADCAST.m_octets;
    }

private:
    std::array<uint8_t, 4> m_octets;
};

}

// Hash implementation
namespace std {

template <>
struct hash<qsox::Ipv4Address> {
    size_t operator()(const qsox::Ipv4Address& addr) const {
        return std::hash<uint32_t>()(addr.toBits());
    }
};

}