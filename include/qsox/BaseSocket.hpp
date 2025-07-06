#pragma once

#include <stdint.h>
#include "SocketAddress.hpp"

namespace qsox {

#ifdef _WIN32
    using SockFd = uintptr_t; // SOCKET is a UINT_PTR on windows which is pointer sized unsigned integer
#else
    using SockFd = int;
#endif

// Base socket class. Applications should not use this.
class BaseSocket {
public:
    using SockFd = qsox::SockFd;

    ~BaseSocket();

    constexpr static inline SockFd InvalidSockFd = -1;

    // Returns the local address the socket is bound to
    NetResult<SocketAddress> localAddress() const;

    // Returns the remote address the socket is connected to (if any)
    NetResult<SocketAddress> remoteAddress() const;

    // Sets the socket to non-blocking mode (or disables it)
    NetResult<> setNonBlocking(bool nonBlocking);

    // Sets the read timeout for the socket. Value of 0 means no timeout.
    NetResult<> setReadTimeout(uint32_t timeoutMs);

    // Sets the write timeout for the socket. Value of 0 means no timeout.
    NetResult<> setWriteTimeout(uint32_t timeoutMs);

    Error getSocketError() const;

    inline SockFd handle() const {
        return m_fd;
    }

protected:
    SockFd m_fd;

    BaseSocket(SockFd fd);
    BaseSocket(const BaseSocket&) = delete;
    BaseSocket& operator=(const BaseSocket&) = delete;

    BaseSocket(BaseSocket&& other) noexcept;
    BaseSocket& operator=(BaseSocket&& other) noexcept;

    NetResult<> _setTimeout(int kind, uint32_t timeoutMs);
};

enum class ShutdownMode {
    Read, Write, Both
};

// Performs initialization of the system socket API (WSAStartup on Windows)
NetResult<> initSockets();

}