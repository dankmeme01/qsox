#if defined(__linux__) && defined(__GLIBC__)
# define _GNU_SOURCE
#endif

#include <qsox/Resolver.hpp>
#include <qsox/Util.hpp>
#include <string.h>

#ifdef _WIN32
# include <ws2tcpip.h>
# include <Windows.h>
# include <stringapiset.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
#endif

namespace qsox::resolver {

#ifdef _WIN32
using qaddrinfo = ADDRINFOEXW;
static void qfree(qaddrinfo* ai) {
    FreeAddrInfoExW(ai);
}

// every damn project i make has to include these functions

static std::string wideToUtf8(std::wstring_view wstr) {
    int count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), &str[0], count, NULL, NULL);
    return str;
}

static std::wstring utf8ToWide(std::string_view str) {
    int count = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), NULL, 0);
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), &wstr[0], count);
    return wstr;
}
#else

using qaddrinfo = struct addrinfo;
static void qfree(qaddrinfo* ai) {
    freeaddrinfo(ai);
}
#endif

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
        case TimedOut:
            return "DNS query timed out";
        case Other:
            return "Other error";
    }

    qsox::unreachable();
}

#ifdef _WIN32
/// GetAddrInfoExW with a timeout on Windows
static Result<qaddrinfo*> gai(const std::string& hostname, int family, int timeoutMs) {
    std::wstring name = utf8ToWide(hostname);

    ADDRINFOEXW hints = {};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_DGRAM;
    ADDRINFOEXW* result = nullptr;

    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;

    int ret = GetAddrInfoExW(name.c_str(), nullptr, NS_ALL, nullptr, &hints, &result, &tv, nullptr, nullptr, nullptr);
    if (ret != 0) {
        return Err(makeError(ret));
    }

    return Ok(result);
}

#elif defined(__linux__) && defined(__GLIBC__)
/// getaddrinfo_a() with a timeout
static Result<qaddrinfo*> gai(const std::string& hostname, int family, int timeoutMs) {
    struct gaicb* reqs[1];
    struct gaicb myReq{};
    qaddrinfo hints{};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_DGRAM;

    myReq.ar_name = hostname.c_str();
    myReq.ar_request = &hints;
    reqs[0] = &myReq;

    int ret = getaddrinfo_a(GAI_NOWAIT, reqs, 1, nullptr);
    if (ret != 0) {
        return Err(makeError(ret));
    }

    struct timespec ts;
    ts.tv_sec = timeoutMs / 1000;
    ts.tv_nsec = (timeoutMs % 1000) * 1000000;

    ret = gai_suspend(reqs, 1, &ts);
    if (ret == EAI_INPROGRESS) {
        // timed out
        gai_cancel(reqs[0]);
        return Err(Error::TimedOut);
    } else if (ret != 0) {
        return Err(makeError(ret));
    }

    // success?
    int error = gai_error(&myReq);
    if (error != 0) {
        return Err(makeError(error));
    }

    return Ok(myReq.ar_result);
}

#else
/// Blocking lookup with no timeout, because this system does not support a timeout.
static Result<qaddrinfo*> gai(const std::string& hostname, int family, int timeoutMs) {
    qaddrinfo hints = {};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_DGRAM;

    qaddrinfo* result = nullptr;

    int res = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (res != 0) {
        return Err(makeError(res));
    }

    return Ok(result);
}
#endif

static qaddrinfo* findFirstAddress(qaddrinfo* list, int family) {
    for (qaddrinfo* ai = list; ai != nullptr; ai = ai->ai_next) {
        if (ai->ai_family == family) {
            return ai;
        }
    }

    return nullptr;
}

template <typename Ip, typename SockAddr, int Family>
Result<Ip> findAndConvert(const std::string& hostname, int timeoutMs) {
    auto addrInfoRes = gai(hostname, Family, timeoutMs);
    if (!addrInfoRes) {
        return Err(addrInfoRes.unwrapErr());
    }

    qaddrinfo* addrInfo = addrInfoRes.unwrap();

    auto qaddr = findFirstAddress(addrInfo, Family);
    if (!qaddr || !qaddr->ai_addr) {
        qfree(addrInfo);
        return Err(Error::NoData);
    }

    auto addr = (SockAddr*) qaddr->ai_addr;
    Ip address;

    if constexpr (std::is_same_v<Ip, Ipv4Address>) {
        address = Ip::fromBits(qsox::byteswap(addr->sin_addr.s_addr));
    } else {
        std::array<uint8_t, 16> octets;
        memcpy(octets.data(), addr->sin6_addr.s6_addr, octets.size());
        address = Ip(octets);
    }

    qfree(addrInfo);
    return Ok(address);
}

Result<Ipv4Address> resolveIpv4(const std::string& hostname, int timeoutMs) {
    return findAndConvert<Ipv4Address, struct sockaddr_in, AF_INET>(hostname, timeoutMs);
}

Result<Ipv6Address> resolveIpv6(const std::string& hostname, int timeoutMs) {
    return findAndConvert<Ipv6Address, struct sockaddr_in6, AF_INET6>(hostname, timeoutMs);
}

Result<IpAddress> resolve(const std::string& hostname, int timeoutMs) {
    auto res = resolveIpv4(hostname, timeoutMs);
    if (res.isOk()) {
        return Ok(IpAddress(res.unwrap()));
    } else {
        return resolveIpv6(hostname, timeoutMs).map([](const Ipv6Address& addr) {
            return IpAddress(addr);
        });
    }
}

} // namespace qsox::resolver,
