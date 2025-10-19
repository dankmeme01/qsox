#include <qsox/TcpStream.hpp>
#include "SocketUtil.hpp"

namespace qsox {

TcpStream::TcpStream(SockFd fd) : BaseSocket(fd) {}

NetResult<TcpStream> TcpStream::connect(const SocketAddress& address, int timeoutMs) {
    SockFd socket;
    GEODE_UNWRAP_INTO(socket, newSocket(address.family(), SOCK_STREAM));

    // create early to take advantage of raii
    TcpStream stream(socket);

    if (timeoutMs > 0) {
        GEODE_UNWRAP(stream.doConnectTimeout(address, timeoutMs));
    } else {
        GEODE_UNWRAP(stream.doConnect(address));
    }

    return Ok(std::move(stream));
}

NetResult<size_t> TcpStream::send(const void* data, size_t size) {
    auto res = ::send(m_fd, reinterpret_cast<const char*>(data), size, sendFlags());
    if (res < 0) {
        return Err(Error::lastOsError());
    } else if (res == 0) {
        return Err(Error::ConnectionClosed);
    }

    return Ok(static_cast<size_t>(res));
}

NetResult<> TcpStream::sendAll(const void* data, size_t size) {
    const char* ptr = static_cast<const char*>(data);
    size_t remaining = size;

    while (remaining > 0) {
        auto result = this->send(ptr, remaining);
        if (result.isErr()) {
            auto err = result.unwrapErr();
#ifdef _WIN32
            return Err(err);
#else
            if (!err.isOsError() || err.osCode() != EINTR) return Err(result.unwrapErr());
            continue; // retry on EINTR
#endif
        }

        size_t sent = result.unwrap();

        if (sent == 0) {
            return Err(Error::ConnectionClosed);
        }

        ptr += sent;
        remaining -= sent;
    }

    return Ok();
}

NetResult<size_t> TcpStream::receive(void* buffer, size_t size) {
    return this->_receive(buffer, size, recvFlags());
}

NetResult<> TcpStream::receiveExact(void* buffer, size_t size) {
    char* ptr = static_cast<char*>(buffer);
    size_t remaining = size;

    while (remaining > 0) {
        auto result = this->receive(ptr, remaining);
        if (result.isErr()) {
            auto err = result.unwrapErr();
#ifdef _WIN32
            return Err(err);
#else
            if (!err.isOsError() || err.osCode() != EINTR) return Err(result.unwrapErr());
            continue; // retry on EINTR
#endif
        }

        size_t received = result.unwrap();

        if (received == 0) {
            return Err(Error::ConnectionClosed);
        }

        ptr += received;
        remaining -= received;
    }

    return Ok();
}

NetResult<size_t> TcpStream::peek(void* buffer, size_t size) {
    return this->_receive(buffer, size, recvFlags() | MSG_PEEK);
}

NetResult<size_t> TcpStream::_receive(void* buffer, size_t size, int flags) {
    auto res = ::recv(m_fd, static_cast<char*>(buffer), size, flags);
    if (res < 0) {
        return Err(Error::lastOsError());
    } else if (res == 0) {
        return Err(Error::ConnectionClosed);
    }

    return Ok(static_cast<size_t>(res));
}

NetResult<void> TcpStream::setNoDelay(bool noDelay) {
    int flag = noDelay ? 1 : 0;
    return mapResult(
        setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&flag), sizeof(flag))
    );
}

SockFd TcpStream::releaseHandle() {
    SockFd fd = m_fd;
    m_fd = InvalidSockFd;
    return fd;
}

}