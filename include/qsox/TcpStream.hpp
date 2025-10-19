#pragma once

#include "BaseSocket.hpp"

namespace qsox {

class TcpStream : public BaseSocket {
public:
    // Creates a new TCP stream, connecting to the given address.
    // Default timeout is 5000ms and can be changed via the `timeoutMs` parameter.
    // If the timeout is set to 0ms, the function may block indefinitely.
    static NetResult<TcpStream> connect(const SocketAddress& address, int timeoutMs = 5000);

    TcpStream(TcpStream&& other) noexcept = default;
    TcpStream& operator=(TcpStream&& other) noexcept = default;

    // Shuts down the TCP stream for reading, writing, or both.
    NetResult<void> shutdown(ShutdownMode mode);

    // Sets the SO_LINGER option on the socket to the given parameters.
    // If enabled, the socket may remain open for the specified timeout after a close operation.
    NetResult<void> setLinger(bool enable, int timeoutMs = 0);

    // Sets the TCP_NODELAY option on the socket, which disables or enables the Nagle algorithm.
    // If `noDelay` is true, small packets are sent immediately without waiting for larger packets to accumulate.
    NetResult<void> setNoDelay(bool noDelay);

    // Sends data over this socket. Returns amount of bytes sent.
    NetResult<size_t> send(const void* data, size_t size);

    // Sends data over this socket, blocking until all data is sent, or an error occurs.
    NetResult<> sendAll(const void* data, size_t size);

    // Receives data from the socket. Returns amount of bytes received.
    NetResult<size_t> receive(void* buffer, size_t size);

    // Receives data from the socket, blocking until the given buffer is full or an error occurs.
    NetResult<> receiveExact(void* buffer, size_t size);

    // Peeks at incoming data without removing it from the queue.
    NetResult<size_t> peek(void* buffer, size_t size);

    // Releases the underlying socket file descriptor, preventing it from being closed on destruction.
    SockFd releaseHandle();

private:
    TcpStream(SockFd fd);

    NetResult<size_t> _receive(void* buffer, size_t size, int flags);
    NetResult<void> doConnect(const SocketAddress& address);
    NetResult<void> doConnectTimeout(const SocketAddress& address, int timeoutMs);

    friend class TcpListener;
};

}