#include <qsox/BaseSocket.hpp>
#include "SocketUtil.hpp"
#include <cassert>

namespace qsox {

BaseSocket::BaseSocket(SockFd fd) : m_fd(fd) {
    assert(m_fd != InvalidSockFd && "Invalid socket file descriptor");
}

BaseSocket::BaseSocket(BaseSocket&& other) noexcept : m_fd(other.m_fd) {
    other.m_fd = InvalidSockFd;
}

BaseSocket& BaseSocket::operator=(BaseSocket&& other) noexcept {
    if (this != &other) {
        if (m_fd != InvalidSockFd) {
            qsox::closeSocket(m_fd);
        }

        m_fd = other.m_fd;
        other.m_fd = InvalidSockFd;
    }

    return *this;
}

BaseSocket::~BaseSocket() {
    if (m_fd != InvalidSockFd) {
        qsox::closeSocket(m_fd);
    }
}

NetResult<SocketAddress> BaseSocket::localAddress() const {
    SockAddrAny storage;
    socklen_t len = storage.maxSize();

    if (getsockname(m_fd, storage.asSockaddr(), &len) < 0) {
        return Err(Error::lastOsError());
    }

    return Ok(storage.toSocketAddress());
}

NetResult<SocketAddress> BaseSocket::remoteAddress() const {
    SockAddrAny storage;
    socklen_t len = storage.maxSize();

    if (getpeername(m_fd, storage.asSockaddr(), &len) < 0) {
        return Err(Error::lastOsError());
    }

    return Ok(storage.toSocketAddress());
}

NetResult<> BaseSocket::setReadTimeout(uint32_t timeoutMs) {
    return this->_setTimeout(SO_RCVTIMEO, timeoutMs);
}

NetResult<> BaseSocket::setWriteTimeout(uint32_t timeoutMs) {
    return this->_setTimeout(SO_SNDTIMEO, timeoutMs);
}

Error BaseSocket::getSocketError() const {
    int err = 0;
    socklen_t len = sizeof(err);

    if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &len) < 0) {
        return Error::lastOsError();
    }

    if (err == 0) {
        return Error::Success;
    }

    return Error::fromOs(err);
}

NetResult<> initSockets() {
    return startupSockets();
}

} // namespace qsox
