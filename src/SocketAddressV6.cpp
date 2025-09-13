#include <qsox/SocketAddressV6.hpp>
#include <qsox/Util.hpp>
#include <fmt/format.h>
#include <charconv>

#ifdef _WIN32
# include <ws2tcpip.h> // sockaddr_in6
#else
# include <arpa/inet.h>
# include <netinet/in.h>
#endif

namespace qsox {

std::string_view SocketAddressV6ParseError::message() const {
    switch (m_code) {
        case InvalidStructure:
            return "Invalid structure for IPv6 socket address (missing square brackets)";
        case InvalidAddress:
            return "Invalid IPv6 address in socket address";
        case MissingPort:
            return "Missing port in socket address";
        case InvalidPort:
            return "Invalid port in socket address";
    }

    qsox::unreachable();
}

static SocketAddressV6ParseError fromIpError(const Ipv6ParseError& error) {
    switch (error.code()) {
        case Ipv6ParseError::Unspecified:
            return SocketAddressV6ParseError::InvalidAddress;
    }

    qsox::unreachable();
}

Result<SocketAddressV6, SocketAddressV6ParseError> SocketAddressV6::parse(std::string_view str) {
    auto colonPos = str.rfind(':');
    if (colonPos == std::string_view::npos || colonPos == 0 || colonPos == str.size() - 1) {
        return Err(SocketAddressV6ParseError::MissingPort);
    }

    std::string_view addressPart = str.substr(0, colonPos);
    std::string_view portPart = str.substr(colonPos + 1);

    // address must be enclosed in square brackets
    if (addressPart.size() < 2 || addressPart.front() != '[' || addressPart.back() != ']') {
        return Err(SocketAddressV6ParseError::InvalidStructure);
    }

    addressPart.remove_prefix(1); // remove '['
    addressPart.remove_suffix(1); // remove ']'

    auto address = Ipv6Address::parse(std::string(addressPart));
    if (address.isErr()) {
        return Err(fromIpError(address.unwrapErr()));
    }

    uint16_t port = 0;
    auto res = std::from_chars(&*portPart.begin(), &*portPart.end(), port);
    if (res.ec != std::errc() || res.ptr != &*portPart.end()) {
        return Err(SocketAddressV6ParseError::InvalidPort);
    }

    return Ok(SocketAddressV6(*address, port));
}

std::string SocketAddressV6::toString() const {
    std::string result = m_address.toString();

    result.push_back(':');
    fmt::format_to(std::back_inserter(result), "{}", m_port);

    return result;
}

void SocketAddressV6::toSockAddr(sockaddr_in6& addr) const {
    addr.sin6_family = AF_INET6;
    m_address.toInAddr(addr.sin6_addr);
    addr.sin6_port = qsox::byteswap(m_port);
    addr.sin6_flowinfo = 0;
    addr.sin6_scope_id = 0;
}

SocketAddressV6 SocketAddressV6::fromSockAddr(const sockaddr_in6& addr) {
    auto address = Ipv6Address::fromInAddr(addr.sin6_addr);
    auto port = qsox::byteswap(addr.sin6_port);
    return SocketAddressV6(address, port);
}

} // namespace qsox
