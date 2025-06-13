#include <qsox/SocketAddressV4.hpp>
#include <qsox/Util.hpp>
#include <fmt/format.h>
#include <charconv>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <arpa/inet.h>
# include <netinet/in.h>
#endif

namespace qsox {

std::string_view SocketAddressV4ParseError::message() const {
    switch (m_code) {
        case MissingOctets:
            return "Missing octets in IPv4 address";
        case TrailingData:
            return "Trailing data after IPv4 address";
        case InvalidOctet:
            return "Invalid octet in IPv4 address";
        case MissingPort:
            return "Missing port in socket address";
        case InvalidPort:
            return "Invalid port in socket address";
    }

    qsox::unreachable();
}

static SocketAddressV4ParseError fromIpv4Error(const Ipv4ParseError& error) {
    using enum Ipv4ParseError::Code;

    switch (error.code()) {
        case MissingOctets:
            return SocketAddressV4ParseError::MissingOctets;
        case TrailingData:
            return SocketAddressV4ParseError::TrailingData;
        case InvalidOctet:
            return SocketAddressV4ParseError::InvalidOctet;
    }

    qsox::unreachable();
}

Result<SocketAddressV4, SocketAddressV4ParseError> SocketAddressV4::parse(std::string_view str) {
    auto colonPos = str.rfind(':');
    if (colonPos == std::string_view::npos || colonPos == 0 || colonPos == str.size() - 1) {
        return Err(SocketAddressV4ParseError::MissingPort);
    }

    std::string_view addressPart = str.substr(0, colonPos);
    std::string_view portPart = str.substr(colonPos + 1);

    auto address = Ipv4Address::parse(addressPart);
    if (address.isErr()) {
        return Err(fromIpv4Error(address.unwrapErr()));
    }

    uint16_t port = 0;
    auto res = std::from_chars(&*portPart.begin(), &*portPart.end(), port);
    if (res.ec != std::errc() || res.ptr != &*portPart.end()) {
        return Err(SocketAddressV4ParseError::InvalidPort);
    }

    return Ok(SocketAddressV4(*address, port));
}

std::string SocketAddressV4::toString() const {
    std::string result = m_address.toString();

    result.push_back(':');
    fmt::format_to(std::back_inserter(result), "{}", m_port);

    return result;
}

void SocketAddressV4::toSockAddr(sockaddr_in& addr) const {
    addr.sin_family = AF_INET;
    m_address.toInAddr(addr.sin_addr);
    addr.sin_port = qsox::byteswap(m_port);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
}

SocketAddressV4 SocketAddressV4::fromSockAddr(const sockaddr_in& addr) {
    auto address = Ipv4Address::fromInAddr(addr.sin_addr);
    auto port = qsox::byteswap(addr.sin_port);
    return SocketAddressV4(address, port);
}

} // namespace qsox
