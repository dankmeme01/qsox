#pragma once

#include "BaseSocket.hpp"
#include "TcpStream.hpp"
#include <stddef.h>
#include <stdint.h>

namespace qsox {

class TcpListener : public BaseSocket {
public:
    // Creates a new TCP listener, binding to the given address
    static NetResult<TcpListener> bind(const SocketAddress& address);

    TcpListener(TcpListener&& other) noexcept = default;
    TcpListener& operator=(TcpListener&& other) noexcept = default;

    // Accepts a new incoming connection, blocking until one is available.
    NetResult<std::pair<TcpStream, SocketAddress>> accept();

private:
    TcpListener(SockFd fd);
};

}