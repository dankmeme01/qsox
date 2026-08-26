#pragma once

#include "Error.hpp"
#include "Ipv4Address.hpp"
#include "Ipv6Address.hpp"

namespace qsox {

QSOX_MAKE_ERROR_STRUCT(IpAddressParseError,
    Unspecified
);

// Class that can hold either an IPv4 or IPv6 address.
class IpAddress {
public:
    constexpr IpAddress(const Ipv4Address& addr) : m_address(addr) {}
    constexpr IpAddress(const Ipv6Address& addr) : m_address(addr) {}

    constexpr IpAddress(const IpAddress& other) = default;
    constexpr IpAddress& operator=(const IpAddress& other) = default;

    constexpr IpAddress& operator=(const Ipv4Address& addr) {
        m_address = addr;
        return *this;
    }

    constexpr IpAddress& operator=(const Ipv6Address& addr) {
        m_address = addr;
        return *this;
    }

    constexpr bool operator==(const IpAddress& other) const {
        return m_address == other.m_address;
    }

    constexpr bool operator!=(const IpAddress& other) const {
        return !(*this == other);
    }

    // Checking

    constexpr bool isV4() const {
        return std::holds_alternative<Ipv4Address>(m_address);
    }

    constexpr bool isV6() const {
        return std::holds_alternative<Ipv6Address>(m_address);
    }

    // Getters

    constexpr const Ipv4Address& asV4() const {
        return std::get<Ipv4Address>(m_address);
    }

    constexpr Ipv4Address& asV4() {
        return std::get<Ipv4Address>(m_address);
    }

    constexpr const Ipv6Address& asV6() const {
        return std::get<Ipv6Address>(m_address);
    }

    constexpr Ipv6Address& asV6() {
        return std::get<Ipv6Address>(m_address);
    }

    // Common methods

    std::string toString() const;
    static Result<IpAddress, void> parse(std::string_view str);

    constexpr bool isLocalhost() const {
        return this->isV4() ? this->asV4().isLocalhost() : this->asV6().isLocalhost();
    }

    constexpr bool isUnspecified() const {
        return this->isV4() ? this->asV4().isUnspecified() : this->asV6().isUnspecified();
    }

private:
    std::variant<Ipv4Address, Ipv6Address> m_address;
    friend struct std::hash<qsox::IpAddress>;
};

} // namespace qsox

// hash
namespace std {

template <>
struct hash<qsox::IpAddress> {
    size_t operator()(const qsox::IpAddress& addr) const {
        return addr.isV4() ? std::hash<qsox::Ipv4Address>()(addr.asV4())
                             : std::hash<qsox::Ipv6Address>()(addr.asV6());
    }
};

} // namespace std