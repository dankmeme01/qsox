#pragma once

#include "Ipv6Address.hpp"

struct sockaddr_in6;

namespace qsox {

// extension to Ipv6ParseError
QSOX_MAKE_ERROR_STRUCT(SocketAddressV6ParseError,
    InvalidStructure,
    InvalidAddress,
    MissingPort,
    InvalidPort,
);

class SocketAddressV6 {
public:
    inline SocketAddressV6() : m_address(Ipv6Address::UNSPECIFIED), m_port(0) {}
    constexpr inline SocketAddressV6(const Ipv6Address& address, uint16_t port = 0) : m_address(address), m_port(port) {}
    constexpr inline SocketAddressV6(const SocketAddressV6& other) = default;
    constexpr inline SocketAddressV6& operator=(const SocketAddressV6& other) = default;

    constexpr inline bool operator==(const SocketAddressV6& other) const {
        return m_address == other.m_address && m_port == other.m_port;
    }

    constexpr inline bool operator!=(const SocketAddressV6& other) const {
        return !(*this == other);
    }

    static constexpr inline SocketAddressV6 any() {
        return SocketAddressV6(Ipv6Address::UNSPECIFIED, 0);
    }

    constexpr inline const Ipv6Address& address() const {
        return m_address;
    }

    constexpr inline Ipv6Address& address() {
        return m_address;
    }

    constexpr inline uint16_t port() const {
        return m_port;
    }

    constexpr inline void setPort(uint16_t port) {
        m_port = port;
    }

    constexpr inline void setAddress(const Ipv6Address& address) {
        m_address = address;
    }

    static Result<SocketAddressV6, SocketAddressV6ParseError> parse(std::string_view str);
    std::string toString() const;
    void toSockAddr(sockaddr_in6& addr) const;
    static SocketAddressV6 fromSockAddr(const sockaddr_in6& addr);

private:
    Ipv6Address m_address;
    uint16_t m_port = 0;

    friend struct std::hash<SocketAddressV6>;
};

} // namespace qsox

// hash
namespace std {

template <>
struct hash<qsox::SocketAddressV6> {
    size_t operator()(const qsox::SocketAddressV6& addr) const {
        size_t h1 = std::hash<qsox::Ipv6Address>()(addr.address());
        size_t h2 = std::hash<uint16_t>()(addr.port());
        return h1 ^ (h2 << 1);
    }
};

} // namespace std
