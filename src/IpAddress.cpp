#include <qsox/IpAddress.hpp>

namespace qsox {

std::string_view IpAddressParseError::message() const {
    switch (m_code) {
        case Unspecified:
            return "Invalid IP address";
    }

    qsox::unreachable();
}

std::string IpAddress::toString() const {
    return this->isV6() ? this->asV6().toString() : this->asV4().toString();
}

Result<IpAddress, void> IpAddress::parse(std::string_view str) {
    // early skip v4 if the address has a colon, IPv4 cannot contain colons and IPv6 must contain at least one
    if (str.find(':') != std::string_view::npos) {
        if (auto v6 = Ipv6Address::parse(str)) {
            return Ok(IpAddress(*v6));
        } else {
            return Err();
        }
    }

    if (auto v4 = Ipv4Address::parse(str)) {
        return Ok(IpAddress(*v4));
    } else if (auto v6 = Ipv6Address::parse(str)) {
        return Ok(IpAddress(*v6));
    }

    return Err();
}

} // namespace qsox
