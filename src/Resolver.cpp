#include <qsox/Resolver.hpp>
#include <qsox/Util.hpp>
#include <string.h>

#ifdef _WIN32
# include <ws2tcpip.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
#endif

namespace qsox::resolver {

std::string_view Error::message() const {
    switch (m_code) {
        case Success:
            return "Success";
        case AddrFamily:
            return "Host does not support the requested address family";
        case TemporaryFailure:
            return "Temporary failure in name resolution";
        case PermanentFailure:
            return "Permanent failure in name resolution";
        case OutOfMemory:
            return "Out of memory";
        case NoData:
            return "No data found for the requested hostname";
        case UnknownHost:
            return "Unknown host";
        case ServiceNotFound:
            return "Service not found";
        case SocketNotSupported:
            return "Socket type not supported";
        case Other:
            return "Other error";
    }

    qsox::unreachable();
}

static Result<struct addrinfo*> gai(const std::string& hostname, int family) {
    struct addrinfo hints = {};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo* result = nullptr;

    int res = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (res != 0) {
        return Err(makeError(res));
    }

    return Ok(result);
}

static struct addrinfo* findFirstAddress(struct addrinfo* list, int family) {
    for (struct addrinfo* ai = list; ai != nullptr; ai = ai->ai_next) {
        if (ai->ai_family == family) {
            return ai;
        }
    }

    return nullptr;
}

template <typename Ip, typename SockAddr, int Family>
Result<Ip> findAndConvert(const std::string& hostname) {
    auto addrInfoRes = gai(hostname, Family);
    if (!addrInfoRes) {
        return Err(addrInfoRes.unwrapErr());
    }

    struct addrinfo* addrInfo = addrInfoRes.unwrap();

    auto addr = (SockAddr*) findFirstAddress(addrInfo, Family)->ai_addr;

    if (!addr) {
        freeaddrinfo(addrInfo);
        return Err(Error::NoData);
    }

    Ip address;

    if constexpr (std::is_same_v<Ip, Ipv4Address>) {
        address = Ip::fromBits(qsox::byteswap(addr->sin_addr.s_addr));
    } else {
        std::array<uint8_t, 16> octets;
        memcpy(octets.data(), addr->sin6_addr.s6_addr, octets.size());
        address = Ip(octets);
    }

    freeaddrinfo(addrInfo);
    return Ok(address);
}

Result<Ipv4Address> resolveIpv4(const std::string& hostname) {
    return findAndConvert<Ipv4Address, struct sockaddr_in, AF_INET>(hostname);
}

Result<Ipv6Address> resolveIpv6(const std::string& hostname) {
    return findAndConvert<Ipv6Address, struct sockaddr_in6, AF_INET6>(hostname);
}

Result<IpAddress> resolve(const std::string& hostname) {
    if (auto addr = resolveIpv4(hostname)) {
        return Ok(IpAddress(*addr));
    } else {
        return resolveIpv6(hostname).map([](const Ipv6Address& addr) {
            return IpAddress(addr);
        });
    }
}

} // namespace qsox::resolver,
