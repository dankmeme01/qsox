#include <qsox/UdpSocket.hpp>
#include "SocketUtil.hpp"

#include <cassert>

namespace qsox {

UdpSocket::UdpSocket(SockFd fd) : BaseSocket(fd) {}

NetResult<UdpSocket> UdpSocket::bind(const SocketAddress& address) {
    return qsox::newBoundSocket(address, SOCK_DGRAM).map([](SockFd fd) {
        return UdpSocket(fd);
    });
}


NetResult<void> UdpSocket::connect(const SocketAddress& address) {
    SockAddrAny addrStorage = address;

    return mapResult(::connect(m_fd, addrStorage.asSockaddr(), addrStorage.size()));
}

NetResult<void> UdpSocket::disconnect() {
    // TODO: is this cross platform?
    struct sockaddr unspec = {};
    unspec.sa_family = AF_UNSPEC;

    return mapResult(::connect(m_fd, &unspec, sizeof(unspec)));
}

NetResult<size_t> UdpSocket::sendTo(const void* buffer, size_t size, const SocketAddress& destination) {
    return this->_sendTo(buffer, size, destination, sendFlags());
}

NetResult<size_t> UdpSocket::send(const void* buffer, size_t size) {
    return this->_send(buffer, size, sendFlags());
}

NetResult<size_t> UdpSocket::recvFrom(void* buffer, size_t size, SocketAddress& sender) {
    return this->_recvFrom(buffer, size, sender, recvFlags());
}

NetResult<size_t> UdpSocket::recvFrom(void* buffer, size_t size, SocketAddressV4& sender) {
    SocketAddress genericAddr = SocketAddressV4{};

    auto result = this->_recvFrom(buffer, size, genericAddr, recvFlags());

    if (!genericAddr.isV4()) {
        return Err(Error::UnsupportedFamily);
    }

    sender = genericAddr.toV4();
    return result;
}

NetResult<size_t> UdpSocket::recvFrom(void* buffer, size_t size, SocketAddressV6& sender) {
    SocketAddress genericAddr = SocketAddressV6{};

    auto result = this->_recvFrom(buffer, size, genericAddr, recvFlags());

    if (!genericAddr.isV6()) {
        return Err(Error::UnsupportedFamily);
    }

    sender = genericAddr.toV6();
    return result;
}

NetResult<size_t> UdpSocket::recv(void* buffer, size_t size) {
    return this->_recv(buffer, size, recvFlags());
}

NetResult<size_t> UdpSocket::peekFrom(void* buffer, size_t size, SocketAddress& sender) {
    return this->_recvFrom(buffer, size, sender, recvFlags() | MSG_PEEK);
}

NetResult<size_t> UdpSocket::peekFrom(void* buffer, size_t size, SocketAddressV4& sender) {
    SocketAddress genericAddr = SocketAddressV4{};

    auto result = this->_recvFrom(buffer, size, genericAddr, recvFlags() | MSG_PEEK);

    if (!genericAddr.isV4()) {
        return Err(Error::UnsupportedFamily);
    }

    sender = genericAddr.toV4();
    return result;
}

NetResult<size_t> UdpSocket::peekFrom(void* buffer, size_t size, SocketAddressV6& sender) {
    SocketAddress genericAddr = SocketAddressV6{};

    auto result = this->_recvFrom(buffer, size, genericAddr, recvFlags() | MSG_PEEK);

    if (!genericAddr.isV6()) {
        return Err(Error::UnsupportedFamily);
    }

    sender = genericAddr.toV6();
    return result;
}

NetResult<size_t> UdpSocket::peek(void* buffer, size_t size) {
    return this->_recv(buffer, size, recvFlags() | MSG_PEEK);
}

NetResult<size_t> UdpSocket::_sendTo(const void* buffer, size_t size, const SocketAddress& destination, int flags) {
    SockAddrAny addrStorage = destination;

    auto sent = ::sendto(m_fd, static_cast<const char*>(buffer), size, flags,
                          addrStorage.asSockaddr(), addrStorage.size());

    if (sent < 0) {
        return Err(Error::lastOsError());
    }

    return Ok(static_cast<size_t>(sent));
}

NetResult<size_t> UdpSocket::_send(const void* buffer, size_t size, int flags) {
    auto sent = ::send(m_fd, static_cast<const char*>(buffer), size, flags);

    if (sent < 0) {
        return Err(Error::lastOsError());
    }

    return Ok(static_cast<size_t>(sent));
}

NetResult<size_t> UdpSocket::_recv(void* buffer, size_t size, int flags) {
    auto received = ::recv(m_fd, static_cast<char*>(buffer), size, flags);

    if (received < 0) {
        return Err(Error::lastOsError());
    }

    return Ok(static_cast<size_t>(received));
}

NetResult<size_t> UdpSocket::_recvFrom(void* buffer, size_t size, SocketAddress& sender, int flags) {
    SockAddrAny addrStorage;
    socklen_t addrLen = addrStorage.maxSize();

    auto received = ::recvfrom(m_fd, static_cast<char*>(buffer), size, flags,
                                  addrStorage.asSockaddr(), &addrLen);

    if (received < 0) {
        return Err(Error::lastOsError());
    }

    sender = addrStorage.toSocketAddress();

    return Ok(static_cast<size_t>(received));
}


} // namespace qsox
