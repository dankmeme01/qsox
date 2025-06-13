#include <qsox/SocketAddress.hpp>
#include <fmt/format.h>
#include <charconv>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <sys/socket.h>
#endif

namespace qsox {

std::string_view SocketAddressParseError::message() const {
    switch (m_code) {
        case InvalidAddress:
            return "Invalid IP address in socket address";
        case MissingPort:
            return "Missing port in socket address";
        case InvalidPort:
            return "Invalid port in socket address";
    }

    qsox::unreachable();
}

std::string SocketAddress::toString() const {
    if (this->isV4()) {
        return fmt::format("{}:{}", m_address.asV4().toString(), m_port);
    } else {
        return fmt::format("[{}]:{}", m_address.asV6().toString(), m_port);
    }
}

Result<SocketAddress, SocketAddressParseError> SocketAddress::parse(std::string_view str) {
    auto colonPos = str.rfind(':');
    if (colonPos == std::string::npos || colonPos == 0 || colonPos == str.size() - 1) {
        return Err(SocketAddressParseError::MissingPort);
    }

    std::string_view addressPart = str.substr(0, colonPos);
    std::string_view portPart = str.substr(colonPos + 1);

    IpAddress outAddr = Ipv4Address::UNSPECIFIED;

    // if the address is enclosed in square brackets, it must be ipv6, and we need to remove them
    if (addressPart.size() >= 2 && addressPart.front() == '[' && addressPart.back() == ']') {
        addressPart.remove_prefix(1);
        addressPart.remove_suffix(1);

        auto addr = Ipv6Address::parse(std::string(addressPart));
        if (!addr) {
            return Err(SocketAddressParseError::InvalidAddress);
        }

        outAddr = *addr;
    } else {
        auto addr = Ipv4Address::parse(std::string(addressPart));
        if (!addr) {
            return Err(SocketAddressParseError::InvalidAddress);
        }

        outAddr = *addr;
    }

    uint16_t port = 0;
    auto res = std::from_chars(&*portPart.begin(), &*portPart.end(), port);
    if (res.ec != std::errc() || res.ptr != &*portPart.end()) {
        return Err(SocketAddressParseError::InvalidPort);
    }

    return Ok(SocketAddress{outAddr, port});
}

int SocketAddress::family() const {
    return this->isV4() ? AF_INET : AF_INET6;
}

}