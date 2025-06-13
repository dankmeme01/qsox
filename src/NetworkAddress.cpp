#include <qsox/NetworkAddress.hpp>
#include <fmt/format.h>
#include <charconv>

namespace qsox {

std::string_view NetworkAddressParseError::message() const {
    switch (m_code) {
        case MissingPort:
            return "Missing host string or port number";
        case InvalidPort:
            return "Invalid port number";
    }

    qsox::unreachable();
}

std::string NetworkAddress::toString() const {
    return fmt::format("{}:{}", m_host, m_port);
}

Result<NetworkAddress, NetworkAddressParseError> NetworkAddress::parse(std::string_view str) {
    auto colonPos = str.rfind(':');
    if (colonPos == std::string_view::npos || colonPos == 0 || colonPos == str.size() - 1) {
        return Err(NetworkAddressParseError::MissingPort);
    }

    std::string host(str.substr(0, colonPos));
    std::string_view portStr = str.substr(colonPos + 1);

    uint16_t port = 0;
    auto res = std::from_chars(&*portStr.begin(), &*portStr.end(), port);
    if (res.ec != std::errc() || res.ptr != &*portStr.end()) {
        return Err(NetworkAddressParseError::InvalidPort);
    }

    return Ok(NetworkAddress(std::move(host), port));
}


Result<SocketAddressV4, resolver::Error> NetworkAddress::resolveV4() const {
    // Try to parse the host as an IPv4 address first
    if (auto addr = Ipv4Address::parse(m_host)) {
        return Ok(SocketAddressV4(*addr, m_port));
    }

    return resolver::resolveIpv4(m_host).map([this](const Ipv4Address& addr) {
        return SocketAddressV4(addr, m_port);
    });
}

Result<SocketAddressV6, resolver::Error> NetworkAddress::resolveV6() const {
    // Try to parse the host as an IPv6 address first
    if (auto addr = Ipv6Address::parse(m_host)) {
        return Ok(SocketAddressV6(*addr, m_port));
    }

    return resolver::resolveIpv6(m_host).map([this](const Ipv6Address& addr) {
        return SocketAddressV6(addr, m_port);
    });
}

Result<SocketAddress, resolver::Error> NetworkAddress::resolve() const {
    // try to parse as ipv4, then ipv6, then resolve to either
    if (auto addr = Ipv4Address::parse(m_host)) {
        return Ok(SocketAddressV4(*addr, m_port));
    }

    if (auto addr = Ipv6Address::parse(m_host)) {
        return Ok(SocketAddressV6(*addr, m_port));
    }

    return resolver::resolve(m_host).map([this](const IpAddress& addr) {
        return SocketAddress(addr, m_port);
    });
}

} // namespace qsox