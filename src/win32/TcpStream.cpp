#include <qsox/TcpStream.hpp>
#include <WinSock2.h>
#include <chrono>
#include "../SocketUtil.hpp"

using hclock = std::chrono::high_resolution_clock;

namespace qsox {

NetResult<void> TcpStream::doConnect(const SocketAddress& address) {
    // convert address
    SockAddrAny addrStorage = address;

    return mapResult(
        ::connect(m_fd, addrStorage.asSockaddr(), addrStorage.size())
    );
}

NetResult<void> TcpStream::doConnectTimeout(const SocketAddress& address, int timeoutMs) {
    if (timeoutMs <= 0) {
        return doConnect(address);
    }

    GEODE_UNWRAP(this->setNonBlocking(true));
    auto result = this->doConnect(address);
    GEODE_UNWRAP(this->setNonBlocking(false));

    if (result.isErr() && result.unwrapErr().osCode() == WSAEWOULDBLOCK) {
        // wait for the socket to become writable
    } else {
        return result;
    }

    struct timeval tv = {};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;

    fd_set writeSet, errorSet;
    FD_ZERO(&writeSet);
    FD_SET(m_fd, &writeSet);
    FD_ZERO(&errorSet);
    FD_SET(m_fd, &errorSet);

    int count = ::select(1, nullptr, &writeSet, &errorSet, &tv);
    if (count < 0) {
        return Err(Error::lastOsError());
    } else if (count == 0) {
        return Err(Error::TimedOut);
    }

    if (writeSet.fd_count != 1) {
        auto err = this->getSocketError();
        if (err != Error::Success) {
            return Err(err);
        }
    }

    return Ok();
}

NetResult<void> TcpStream::setLinger(bool enable, int timeoutMs) {
    LINGER lg = {};
    lg.l_onoff = enable ? 1 : 0;
    lg.l_linger = timeoutMs / 1000; // convert ms to seconds

    return mapResult(setsockopt(m_fd, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>(&lg), sizeof(lg)));
}

NetResult<void> TcpStream::shutdown(ShutdownMode mode) {
    int how;
    switch (mode) {
        case ShutdownMode::Read:
            how = SD_SEND;
            break;
        case ShutdownMode::Write:
            how = SD_RECEIVE;
            break;
        case ShutdownMode::Both:
            how = SD_BOTH;
            break;
        default:
            return Err(Error::InvalidArgument);
    }

    return mapResult(::shutdown(m_fd, how));
}

}