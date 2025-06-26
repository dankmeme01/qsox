#pragma once

#include <qsox/SocketAddress.hpp>
#include <qsox/IpAddress.hpp>
#include <qsox/Util.hpp>
#include <qsox/BaseSocket.hpp>
#include <algorithm>

#ifdef _WIN32
# include <ws2tcpip.h>
#else
# include <netinet/in.h>
# include <netinet/tcp.h> // for TCP_NODELAY
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

static inline int recvFlags() {
    return 0;
}

static inline int sendFlags() {
#ifdef _WIN32
    return 0;
#else
    return MSG_NOSIGNAL;
#endif
}

namespace qsox {

// Utils for conversion between SocketAddress(V4/V6) and sockaddr(_in/_in6)

// This is like sockaddr_storage, but it can only hold IPv4 and IPv6 addresses,
// and is thus a lot smaller (compared to ~200 bytes for sockaddr_storage)
union SockAddrAny {
    sockaddr_in v4;
    sockaddr_in6 v6;

    // Initialization / copy

    SockAddrAny() {
        memset((void*) this, 0, sizeof(SockAddrAny));
    }

    SockAddrAny& operator=(const SockAddrAny& other) {
        if (this != &other) {
            memcpy((void*) this, &other, sizeof(SockAddrAny));
        }

        return *this;
    }

    SockAddrAny(const SockAddrAny& other) {
        *this = other;
    }

    // Assignment from raw C types

    SockAddrAny& operator=(const sockaddr_in& addr) {
        v4 = addr;
        v4.sin_family = AF_INET;
        return *this;
    }

    SockAddrAny(const sockaddr_in& addr) {
        *this = addr;
    }

    SockAddrAny& operator=(const sockaddr_in6& addr) {
        v6 = addr;
        v6.sin6_family = AF_INET6;
        return *this;
    }

    SockAddrAny(const sockaddr_in6& addr) {
        *this = addr;
    }

    // Assignment from SocketAddress

    SockAddrAny& operator=(const SocketAddressV4& addr) {
        addr.toSockAddr(v4);
        return *this;
    }

    SockAddrAny(const SocketAddressV4& addr) {
        *this = addr;
    }

    SockAddrAny& operator=(const SocketAddressV6& addr) {
        addr.toSockAddr(v6);
        return *this;
    }

    SockAddrAny(const SocketAddressV6& addr) {
        *this = addr;
    }

    SockAddrAny& operator=(const SocketAddress& addr) {
        if (addr.isV4()) {
            return *this = addr.toV4();
        } else if (addr.isV6()) {
            return *this = addr.toV6();
        } else {
            qsox::unreachable();
        }
    }

    SockAddrAny(const SocketAddress& addr) {
        *this = addr;
    }

    // Accessors / converters

    size_t size() const {
        return (family() == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    }

    size_t maxSize() const {
        return sizeof(SockAddrAny);
    }

    sockaddr_in& asV4() {
        return v4;
    }

    const sockaddr_in& asV4() const {
        return v4;
    }

    sockaddr_in6& asV6() {
        return v6;
    }

    const sockaddr_in6& asV6() const {
        return v6;
    }

    sockaddr* asSockaddr() {
        return reinterpret_cast<sockaddr*>(this);
    }

    const sockaddr* asSockaddr() const {
        return reinterpret_cast<const sockaddr*>(this);
    }

    int family() const {
        return v4.sin_family;
    }

    SocketAddressV4 toSocketAddressV4() const {
        return SocketAddressV4::fromSockAddr(v4);
    }

    SocketAddressV6 toSocketAddressV6() const {
        return SocketAddressV6::fromSockAddr(v6);
    }

    SocketAddress toSocketAddress() const {
        if (family() == AF_INET) {
            return toSocketAddressV4();
        } else if (family() == AF_INET6) {
            return toSocketAddressV6();
        } else {
            qsox::unreachable();
        }
    }
};

// Socket closing
inline void closeSocket(auto socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

struct _SocketStartup {
    _SocketStartup() : error(Error::Success) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            error = Error::lastOsError();
        }
#endif
    }

    ~_SocketStartup() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool _successful;
    Error error;
};

inline NetResult<> startupSockets() {
    static _SocketStartup startup;

    if (startup.error.code() != Error::Success) {
        return Err(startup.error);
    }

    return Ok();
}

// Creates a new socket, guarantees on success that the return value is not -1
inline NetResult<BaseSocket::SockFd> newSocket(int family, int type, int protocol = 0) {
    auto startupRes = startupSockets();
    if (!startupRes) {
        return Err(startupRes.unwrapErr());
    }

#ifdef _WIN32
    auto sock = WSASocketW(family, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
    auto sock = socket(family, type, protocol);
#endif

    if (sock == BaseSocket::InvalidSockFd) {
        return Err(Error::lastOsError());
    }

    return Ok(sock);
}

inline NetResult<BaseSocket::SockFd> newBoundSocket(const SocketAddress& address, int type, int protocol = 0) {
    auto newRes = newSocket(address.family(), type, protocol);
    if (!newRes) {
        return Err(newRes.unwrapErr());
    }

    auto sock = newRes.unwrap();

    SockAddrAny addrStorage = address;

    // explicitly disable v6 only mode for v6 sockets
#ifdef IPV6_V6ONLY
    if (address.isV6()) {
        int v6only = 0;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&v6only), sizeof(v6only)) < 0) {
            qsox::closeSocket(sock);
            return Err(Error::lastOsError());
        }
    }
#endif

    if (::bind(sock, addrStorage.asSockaddr(), addrStorage.size()) < 0) {
        qsox::closeSocket(sock);
        return Err(Error::lastOsError());
    }

    return Ok(sock);
}

} // namespace qsox

