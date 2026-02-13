#include <qsox/Ipv4Address.hpp>
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

const Ipv4Address Ipv4Address::LOCALHOST{127, 0, 0, 1};
const Ipv4Address Ipv4Address::UNSPECIFIED{0, 0, 0, 0};
const Ipv4Address Ipv4Address::BROADCAST{255, 255, 255, 255};

std::string_view Ipv4ParseError::message() const {
    switch (m_code) {
        case Code::MissingOctets:
            return "Missing octets in IPv4 address";
        case Code::TrailingData:
            return "Trailing data in IPv4 address";
        case Code::InvalidOctet:
            return "Invalid octet in IPv4 address";
    }

    qsox::unreachable();
}

Result<Ipv4Address, Ipv4ParseError> Ipv4Address::parse(std::string_view str) {
    Ipv4Address out;

    for (size_t i = 0; i < 4; i++) {
        size_t nextDot = str.find('.');
        if (nextDot == std::string_view::npos) {
            if (i < 3) {
                return Err(Ipv4ParseError::MissingOctets);
            }

            nextDot = str.size(); // last octet
        }

        std::string_view octetStr = str.substr(0, nextDot);

        uint8_t octet;
        auto result = std::from_chars(&*octetStr.begin(), &*octetStr.end(), octet);

        if (result.ec != std::errc()) {
            return Err(Ipv4ParseError::InvalidOctet);
        }

        out.m_octets[i] = octet;

        if (i < 3) {
            str.remove_prefix(nextDot + 1); // move past the dot
        } else {
            str.remove_prefix(nextDot); // no dot
        }
    }

    if (!str.empty()) {
        return Err(Ipv4ParseError::TrailingData);
    }

    return Ok(out);
}

std::string Ipv4Address::toString() const {
    std::string str;
    str.reserve(15);

    for (size_t i = 0; i < 4; i++) {
        fmt::format_to(std::back_inserter(str), "{}", m_octets[i]);

        if (i < 3) {
            str.push_back('.');
        }
    }

    return str;
}

void Ipv4Address::toInAddr(struct in_addr& addr) const {
    addr.s_addr = qsox::byteswap(toBits());
}

Ipv4Address Ipv4Address::fromInAddr(const in_addr& addr) {
    return Ipv4Address::fromBits(qsox::byteswap(addr.s_addr));
}

} // namespace qsox
