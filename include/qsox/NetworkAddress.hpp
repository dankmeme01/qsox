#pragma once

#include "SocketAddress.hpp"
#include "Resolver.hpp"
#include <stdint.h>
#include <string>

namespace qsox {

QSOX_MAKE_ERROR_STRUCT(NetworkAddressParseError,
    MissingPort,
    InvalidPort,
);

// NetworkAddress is a class that represents an endpoint on the internet or on a network.
// It is similar to SocketAddress, but instead of holding an IP address, it holds an opaque string identifying the host.
// The host can be an IPv4 or IPv6 address, as well as a domain name. It is only resolved to an IP address when needed.
class NetworkAddress {
public:
    NetworkAddress(std::string host, uint16_t port) : m_host(std::move(host)), m_port(port) {}
    NetworkAddress(const NetworkAddress&) = default;
    NetworkAddress& operator=(const NetworkAddress&) = default;
    NetworkAddress(NetworkAddress&&) noexcept = default;
    NetworkAddress& operator=(NetworkAddress&&) noexcept = default;

    // allow construction from a SocketAddress
    NetworkAddress(const SocketAddress& address) : m_host(address.address().toString()), m_port(address.port()) {}
    NetworkAddress& operator=(const SocketAddress& address) {
        m_host = address.address().toString();
        m_port = address.port();
        return *this;
    }

    // allow construction from an IpAddress and port
    NetworkAddress(const IpAddress& address, uint16_t port) : m_host(address.toString()), m_port(port) {}

    // comparison operators
    bool operator==(const NetworkAddress& other) const {
        return m_host == other.m_host && m_port == other.m_port;
    }

    bool operator!=(const NetworkAddress& other) const {
        return !(*this == other);
    }

    // setters
    void setHost(const std::string& newm_host) {
        m_host = newm_host;
    }

    void setPort(uint16_t newPort) {
        m_port = newPort;
    }

    // getters

    const std::string& host() const {
        return m_host;
    }

    uint16_t port() const {
        return m_port;
    }

    // Converts the address to a string.
    // Does not perform any DNS resolution, simply returns 'host:port'.
    std::string toString() const;

    // Parses a string in the format "host:port" into a NetworkAddress
    static Result<NetworkAddress, NetworkAddressParseError> parse(std::string_view str);

    // Resolves the address to a SocketAddressV4
    Result<SocketAddressV4, resolver::Error> resolveV4() const;

    // Resolves the address to a SocketAddressV6
    Result<SocketAddressV6, resolver::Error> resolveV6() const;

    // Resolves the address to a SocketAddress. Currently, this will prioritize IPv4 and return a
    // SocketAddressV4 unless an IPv4 address is unavailable or the host string is an IPv6 address.
    Result<SocketAddress, resolver::Error> resolve() const;

private:
    std::string m_host;
    uint16_t m_port;
    friend struct std::hash<NetworkAddress>;
};

} // namespace qsox

// hash
namespace std {

template <>
struct hash<qsox::NetworkAddress> {
    size_t operator()(const qsox::NetworkAddress& addr) const {
        return std::hash<std::string>()(addr.m_host) ^ (std::hash<uint16_t>()(addr.m_port) << 1);
    }
};

} // namespace std