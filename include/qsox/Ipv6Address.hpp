#pragma once

#include <array>
#include <compare>
#include <stdint.h>
#include <stddef.h>
#include <optional>
#include "Ipv4Address.hpp"
#include "Error.hpp"

struct in6_addr;

namespace qsox {

QSOX_MAKE_ERROR_STRUCT(Ipv6ParseError,
    Unspecified
);

class Ipv6Address {
public:
    // Represents ::1
    static Ipv6Address LOCALHOST;

    // Represents :: (unspecified address)
    static Ipv6Address UNSPECIFIED;

    constexpr inline Ipv6Address() : m_octets{} {}
    constexpr inline Ipv6Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                                 uint8_t e, uint8_t f, uint8_t g, uint8_t h,
                                 uint8_t i, uint8_t j, uint8_t k, uint8_t l,
                                 uint8_t m, uint8_t n, uint8_t o, uint8_t p)
        : m_octets{a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p} {}
    constexpr inline Ipv6Address(const std::array<uint8_t, 16>& octets) : m_octets(octets) {}
    constexpr inline Ipv6Address(const Ipv6Address& other) = default;
    constexpr inline Ipv6Address& operator=(const Ipv6Address& other) = default;
    Ipv6Address(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f, uint16_t g, uint16_t h);

    constexpr inline uint8_t operator[](size_t index) const {
        return m_octets[index];
    }

    constexpr inline uint8_t& operator[](size_t index) {
        return m_octets[index];
    }

    constexpr inline bool operator==(const Ipv6Address& other) const {
        return m_octets == other.m_octets;
    }

    constexpr inline bool operator!=(const Ipv6Address& other) const {
        return !(*this == other);
    }

    constexpr inline std::strong_ordering operator<=>(const Ipv6Address& rhs) const {
        if (m_octets == rhs.m_octets) {
            return std::strong_ordering::equal;
        }

        return m_octets < rhs.m_octets ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    constexpr inline const std::array<uint8_t, 16>& octets() const {
        return m_octets;
    }

    constexpr inline std::array<uint8_t, 16>& octets() {
        return m_octets;
    }

    std::array<uint16_t, 8> segments() const;

    static Result<Ipv6Address, Ipv6ParseError> parse(const std::string& str);
    std::string toString() const;
    void toInAddr(in6_addr& addr) const;
    static Ipv6Address fromInAddr(const in6_addr& addr);

    // Converts this address to an Ipv4-mapped address if applicable. (e.g. ::ffff:a.b.c.d becomes a.b.c.d)
    // If the address does not start with ::ffff, it will return std::nullopt.
    std::optional<Ipv4Address> toIpv4Mapped() const;

    static Ipv6Address fromIpv4Mapped(const Ipv4Address& addr);

    // Returns whether the address is unspecified (::)
    constexpr inline bool isUnspecified() const {
        return m_octets == UNSPECIFIED.m_octets;
    }

    // Returns whether the address is localhost (::1)
    constexpr inline bool isLocalhost() const {
        return m_octets == LOCALHOST.m_octets;
    }

    constexpr inline bool isUniqueLocal() const {
        // Unique Local Addresses (ULA) are in the range fc00::/7
        return (m_octets[0] & 0xFE) == 0xFC;
    }

protected:
    std::array<uint8_t, 16> m_octets;
};

} // namespace qsox

// Hash implementation
namespace std {

template <>
struct hash<qsox::Ipv6Address> {
    size_t operator()(const qsox::Ipv6Address& addr) const {
        std::hash<std::string_view> hasher;
        return hasher(std::string_view(reinterpret_cast<const char*>(addr.octets().data()), 16));
    }
};

}