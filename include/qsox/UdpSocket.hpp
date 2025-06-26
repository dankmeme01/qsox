#pragma once

#include "BaseSocket.hpp"
#include <stddef.h>
#include <stdint.h>

namespace qsox {

class UdpSocket : public BaseSocket {
public:
    // Creates a new UDP socket, binding to the given address
    static NetResult<UdpSocket> bind(const SocketAddress& address);
    // Creates a new UDP socket, binding to 0.0.0.0 and a random port
    static NetResult<UdpSocket> bindAny(bool ipv6 = true);

    UdpSocket(UdpSocket&& other) noexcept = default;
    UdpSocket& operator=(UdpSocket&& other) noexcept = default;

    // Connects the UDP socket to a remote address
    // This sets the default destination for send operations, and limits the packets received with 'recv()' to that address.
    NetResult<void> connect(const SocketAddress& address);

    // Disconnects the UDP socket from the remote address
    NetResult<void> disconnect();

    // Sends a datagram to the specified address. Returns the number of bytes sent.
    NetResult<size_t> sendTo(const void* buffer, size_t size, const SocketAddress& destination);

    // Sends a datagram to the connected address. Returns the number of bytes sent.
    // Will fail if the socket is not connected.
    NetResult<size_t> send(const void* buffer, size_t size);

    // Receives a single datagram from the socket. If the buffer is too small, excess data is discarded.
    // On success, returns the number of bytes received.
    NetResult<size_t> recvFrom(void* buffer, size_t size, SocketAddress& sender);

    // Receives a single datagram from the socket. If the buffer is too small, excess data is discarded.
    // On success, returns the number of bytes received.
    NetResult<size_t> recvFrom(void* buffer, size_t size, SocketAddressV4& sender);

    // Receives a single datagram from the socket. If the buffer is too small, excess data is discarded.
    // On success, returns the number of bytes received.
    NetResult<size_t> recvFrom(void* buffer, size_t size, SocketAddressV6& sender);

    // Receives a single datagram from the connected address. If the buffer is too small, excess data is discarded.
    // On success returns the number of bytes received, will fail if the socket is not connected.
    NetResult<size_t> recv(void* buffer, size_t size);

    // Peeks at the next datagram in the socket without removing it from the queue.
    NetResult<size_t> peekFrom(void* buffer, size_t size, SocketAddress& sender);

    // Peeks at the next datagram in the socket without removing it from the queue.
    NetResult<size_t> peekFrom(void* buffer, size_t size, SocketAddressV4& sender);

    // Peeks at the next datagram in the socket without removing it from the queue.
    NetResult<size_t> peekFrom(void* buffer, size_t size, SocketAddressV6& sender);

    // Peeks at the next datagram in the connected socket without removing it from the queue.
    // Will fail if the socket is not connected.
    NetResult<size_t> peek(void* buffer, size_t size);

private:
    UdpSocket(SockFd fd);

    NetResult<size_t> _sendTo(const void* buffer, size_t size, const SocketAddress& destination, int flags);
    NetResult<size_t> _send(const void* buffer, size_t size, int flags);
    NetResult<size_t> _recv(void* buffer, size_t size, int flags);
    NetResult<size_t> _recvFrom(void* buffer, size_t size, SocketAddress& sender, int flags);
};

} // namespace qsox
