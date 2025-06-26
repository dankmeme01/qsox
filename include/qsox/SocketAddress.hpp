#pragma once

#include "Error.hpp"
#include "SocketAddressV4.hpp"
#include "SocketAddressV6.hpp"
#include "IpAddress.hpp"

namespace qsox {

QSOX_MAKE_ERROR_STRUCT(SocketAddressParseError,
    InvalidAddress,
    MissingPort,
    InvalidPort,
);

// Socket address that contains an Ipv4/Ipv6 address and a port.
class SocketAddress {
public:
    constexpr inline SocketAddress(const IpAddress& addr, uint16_t port = 0) : m_address(addr), m_port(port) {}
    constexpr inline SocketAddress(const SocketAddressV4& addr) : m_address(addr.address()), m_port(addr.port()) {}
    constexpr inline SocketAddress(const SocketAddressV6& addr) : m_address(addr.address()), m_port(addr.port()) {}

    constexpr inline SocketAddress(const SocketAddress& other) = default;
    constexpr inline SocketAddress& operator=(const SocketAddress& other) = default;

    static constexpr inline SocketAddress any(bool v6 = true) {
        if (v6) {
            return SocketAddress(SocketAddressV6::any());
        } else {
            return SocketAddress(SocketAddressV4::any());
        }
    }

    // allow assignment from SocketAddressV4 and SocketAddressV6
    constexpr inline SocketAddress& operator=(const SocketAddressV4& addr) {
        m_address = addr.address();
        m_port = addr.port();
        return *this;
    }

    constexpr inline SocketAddress& operator=(const SocketAddressV6& addr) {
        m_address = addr.address();
        m_port = addr.port();
        return *this;
    }

    // comparison operators

    constexpr inline bool operator==(const SocketAddress& other) const {
        return m_address == other.m_address && m_port == other.m_port;
    }

    constexpr inline bool operator!=(const SocketAddress& other) const {
        return !(*this == other);
    }

    // comparison with SocketAddressV4 and SocketAddressV6

    constexpr inline bool operator==(const SocketAddressV4& other) const {
        return m_address.isV4() && m_address.asV4() == other.address() && m_port == other.port();
    }

    constexpr inline bool operator!=(const SocketAddressV4& other) const {
        return !(*this == other);
    }

    constexpr inline bool operator==(const SocketAddressV6& other) const {
        return m_address.isV6() && m_address.asV6() == other.address() && m_port == other.port();
    }

    constexpr inline bool operator!=(const SocketAddressV6& other) const {
        return !(*this == other);
    }

    // Checking

    constexpr inline bool isV4() const {
        return m_address.isV4();
    }

    constexpr inline bool isV6() const {
        return m_address.isV6();
    }

    // Getters

    constexpr inline const IpAddress& address() const {
        return m_address;
    }

    constexpr inline IpAddress& address() {
        return m_address;
    }

    constexpr inline uint16_t port() const {
        return m_port;
    }

    constexpr inline void setPort(uint16_t port) {
        m_port = port;
    }

    constexpr inline void setAddress(const IpAddress& address) {
        m_address = address;
    }

    constexpr inline SocketAddressV4 toV4() const {
        return SocketAddressV4(m_address.asV4(), m_port);
    }

    constexpr inline SocketAddressV6 toV6() const {
        return SocketAddressV6(m_address.asV6(), m_port);
    }

    constexpr inline IpAddress ip() const {
        return m_address;
    }

    // Parsing/formatting

    std::string toString() const;
    static Result<SocketAddress, SocketAddressParseError> parse(std::string_view str);

    // Returns the address family (AF_INET or AF_INET6)
    int family() const;

private:
    IpAddress m_address;
    uint16_t m_port = 0;

    friend struct std::hash<SocketAddress>;
};

} // namespace qsox

// hash
namespace std {

template <>
struct hash<qsox::SocketAddress> {
    size_t operator()(const qsox::SocketAddress& addr) const {
        size_t h1 = std::hash<qsox::IpAddress>()(addr.m_address);
        size_t h2 = std::hash<uint16_t>()(addr.m_port);
        return h1 ^ (h2 << 1);
    }
};

} // namespace std
