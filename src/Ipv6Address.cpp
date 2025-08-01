#include <qsox/Ipv6Address.hpp>
#include <qsox/Util.hpp>
#include <fmt/format.h>

#ifdef _WIN32
# include <ws2tcpip.h> // inet_pton
#else
# include <arpa/inet.h> // inet_pton
#endif

namespace qsox {

Ipv6Address Ipv6Address::LOCALHOST{0, 0, 0, 0, 0, 0, 0, 1};
Ipv6Address Ipv6Address::UNSPECIFIED{0, 0, 0, 0, 0, 0, 0, 0};

std::string_view Ipv6ParseError::message() const {
    switch (m_code) {
        case Code::Unspecified:
            return "Error parsing IPv6 address";
    }

    qsox::unreachable();
}

Ipv6Address::Ipv6Address(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f, uint16_t g, uint16_t h) {
    // Convert 16-bit groups to big endian representations
    std::array<uint16_t, 8> groups = {
        qsox::byteswap(a),
        qsox::byteswap(b),
        qsox::byteswap(c),
        qsox::byteswap(d),
        qsox::byteswap(e),
        qsox::byteswap(f),
        qsox::byteswap(g),
        qsox::byteswap(h)
    };

    std::array<uint8_t, 16> octets;
    memcpy(octets.data(), groups.data(), sizeof(groups));
}

std::array<uint16_t, 8> Ipv6Address::segments() const {
    std::array<uint16_t, 8> segments;
    memcpy(segments.data(), m_octets.data(), sizeof(segments));

    // convert to native endian
    for (auto& segment : segments) {
        segment = qsox::byteswap(segment);
    }

    return segments;
}

std::optional<Ipv4Address> Ipv6Address::toIpv4Mapped() const {
    // first 10 octets must be 0, next 2 must be 0xff, then the last 4 octets must be an IPv4 address

    for (size_t i = 0; i < 10; i++) {
        if (m_octets[i] != 0) {
            return std::nullopt; // not an IPv4-mapped address
        }
    }

    if (m_octets[10] != 0xff || m_octets[11] != 0xff) {
        return std::nullopt; // not an IPv4-mapped address
    }

    return Ipv4Address(
        m_octets[12],
        m_octets[13],
        m_octets[14],
        m_octets[15]
    );
}

Ipv6Address Ipv6Address::fromIpv4Mapped(const Ipv4Address& addr) {
    std::array<uint8_t, 16> octets = {};
    octets[10] = 0xff;
    octets[11] = 0xff;
    octets[12] = addr[0];
    octets[13] = addr[1];
    octets[14] = addr[2];
    octets[15] = addr[3];
    return Ipv6Address(octets);
}

Result<Ipv6Address, Ipv6ParseError> Ipv6Address::parse(const std::string& str) {
    // Parsing an IPv6 address is significantly more complex than ipv4, so let's just use inet_pton for now

    Ipv6Address addr;
    if (inet_pton(AF_INET6, str.c_str(), addr.m_octets.data()) != 1) {
        return Err(Ipv6ParseError::Unspecified);
    }

    return Ok(addr);
}

std::string Ipv6Address::toString() const {
    auto mappedV4 = this->toIpv4Mapped();
    if (mappedV4) {
        return fmt::format("::ffff:{}", mappedV4->toString());
    }

    auto segments = this->segments();

    // find the longest sequence of zero segments
    struct Span {
        size_t start;
        size_t length;
    };

    Span longest{}, current{};

    for (size_t i = 0; i < segments.size(); i++) {
        uint16_t segment = segments[i];

        if (segment == 0) {
            if (current.length == 0) {
                current.start = i;
            }

            current.length++;

            if (current.length > longest.length) {
                longest = current;
            }
        } else {
            current = Span{};
        }
    }

    std::string out;
    out.reserve(39);

    auto writeSubslice = [&](size_t start, size_t count) {
        if (count == 0) {
            return;
        }

        for (size_t i = start; i < start + count; i++) {
            if (i > start) {
                out.append(":");
            }

            fmt::format_to(std::back_inserter(out), "{:x}", segments[i]);
        }
    };

    if (longest.length > 1) {
        writeSubslice(0, longest.start);
        out.append("::");
        writeSubslice(longest.start + longest.length, segments.size() - (longest.start + longest.length));
    } else {
        writeSubslice(0, segments.size());
    }

    return out;
}

void Ipv6Address::toInAddr(in6_addr& addr) const {
    memcpy(addr.s6_addr, m_octets.data(), sizeof(addr.s6_addr));
}

Ipv6Address Ipv6Address::fromInAddr(const in6_addr& addr) {
    Ipv6Address result;
    memcpy(result.m_octets.data(), addr.s6_addr, sizeof(addr.s6_addr));
    return result;
}

}