#pragma once

#include "Ipv4Address.hpp"

struct sockaddr_in;

namespace qsox {

// extension to Ipv4ParseError
QSOX_MAKE_ERROR_STRUCT(SocketAddressV4ParseError,
    MissingOctets,
    TrailingData,
    InvalidOctet,
    MissingPort,
    InvalidPort,
);

class SocketAddressV4 {
public:
    constexpr SocketAddressV4() : m_address(Ipv4Address{}), m_port(0) {}
    constexpr SocketAddressV4(const Ipv4Address& address, uint16_t port = 0) : m_address(address), m_port(port) {}
    constexpr SocketAddressV4(const SocketAddressV4& other) = default;
    constexpr SocketAddressV4& operator=(const SocketAddressV4& other) = default;

    constexpr bool operator==(const SocketAddressV4& other) const {
        return m_address == other.m_address && m_port == other.m_port;
    }

    constexpr bool operator!=(const SocketAddressV4& other) const {
        return !(*this == other);
    }

    static constexpr SocketAddressV4 any() {
        return SocketAddressV4(Ipv4Address::UNSPECIFIED, 0);
    }

    constexpr const Ipv4Address& address() const {
        return m_address;
    }

    constexpr Ipv4Address& address() {
        return m_address;
    }

    constexpr uint16_t port() const {
        return m_port;
    }

    constexpr void setPort(uint16_t port) {
        m_port = port;
    }

    constexpr void setAddress(const Ipv4Address& address) {
        m_address = address;
    }

    static Result<SocketAddressV4, SocketAddressV4ParseError> parse(std::string_view str);
    std::string toString() const;
    void toSockAddr(sockaddr_in& addr) const;
    static SocketAddressV4 fromSockAddr(const sockaddr_in& addr);

private:
    Ipv4Address m_address;
    uint16_t m_port = 0;

    friend struct std::hash<SocketAddressV4>;
};

} // namespace qsox

// hash
namespace std {

template <>
struct hash<qsox::SocketAddressV4> {
    size_t operator()(const qsox::SocketAddressV4& addr) const {
        size_t h1 = std::hash<qsox::Ipv4Address>()(addr.address());
        size_t h2 = std::hash<uint16_t>()(addr.port());
        return h1 ^ (h2 << 1);
    }
};

} // namespace std
