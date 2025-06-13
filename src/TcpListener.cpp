#include <qsox/TcpListener.hpp>
#include "SocketUtil.hpp"

namespace qsox {

TcpListener::TcpListener(SockFd fd) : BaseSocket(fd) {}

NetResult<TcpListener> TcpListener::bind(const SocketAddress& address) {
    auto sockRes = qsox::newBoundSocket(address, SOCK_STREAM);
    if (!sockRes) {
        return Err(sockRes.unwrapErr());
    }

    auto sock = sockRes.unwrap();

    // Rust uses 1024
    if (listen(sock, 1024) < 0) {
        qsox::closeSocket(sock);
        return Err(Error::lastOsError());
    }

    return Ok(TcpListener(sock));
}

NetResult<std::pair<TcpStream, SocketAddress>> TcpListener::accept() {
    SockAddrAny addrStorage;
    socklen_t addrLen = addrStorage.maxSize();

    SockFd clientSock = ::accept(m_fd, addrStorage.asSockaddr(), &addrLen);

    if (clientSock == InvalidSockFd) {
        return Err(Error::lastOsError());
    }

    // create early and take advantage of raii in case of an error
    TcpStream stream(clientSock);

    return Ok(std::make_pair(std::move(stream), addrStorage.toSocketAddress()));
}

} // namespace qsox
