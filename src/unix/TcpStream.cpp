#include <qsox/TcpStream.hpp>
#include <sys/poll.h>
#include <chrono>
#include "../SocketUtil.hpp"

using hclock = std::chrono::high_resolution_clock;

namespace qsox {

NetResult<void> TcpStream::doConnect(const SocketAddress& address, bool nonBlocking) {
    // convert address
    SockAddrAny addrStorage = address;

    if (nonBlocking) {
        GEODE_UNWRAP(this->setNonBlocking(true));
    }

    // attempt to connect
    while (true) {
        int cres = ::connect(m_fd, addrStorage.asSockaddr(), addrStorage.size());

        if (cres == -1) {
            if (nonBlocking && errno == EINPROGRESS) {
                return Ok();
            }

            switch(errno) {
                case EINTR: continue;
                case EISCONN: return Ok();
                default:
                    return Err(Error::lastOsError());
            }
        }

        return Ok();
    }
}

NetResult<void> TcpStream::doConnectTimeout(const SocketAddress& address, int timeoutMs) {
    if (timeoutMs <= 0) {
        return doConnect(address, false);
    }

    SockAddrAny addrStorage = address;

    GEODE_UNWRAP(this->setNonBlocking(true));
    int cres = ::connect(m_fd, addrStorage.asSockaddr(), addrStorage.size());
    GEODE_UNWRAP(this->setNonBlocking(false));

    if (cres != -1) {
        return Ok();
    }

    switch (errno) {
        case EINPROGRESS: break;
        default:
            return Err(Error::lastOsError());
    }

    struct pollfd fd = {};
    fd.fd = m_fd;
    fd.events = POLLOUT;
    fd.revents = 0;

    auto start = hclock::now();

    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(hclock::now() - start).count();

        if (elapsed >= timeoutMs) {
            return Err(Error::TimedOut);
        }

        int time = timeoutMs - static_cast<int>(elapsed);

        int pollRes = ::poll(&fd, 1, time);

        // rust does nothing for 0, checks for errno on -1, and does revents check on any other values
        if (pollRes == -1) {
            if (errno != EINTR) {
                return Err(Error::lastOsError());
            }
        } else if (pollRes != 0) {
            if ((fd.revents & (POLLHUP | POLLERR)) != 0) {
                return Err(this->getSocketError());
            }

            return Ok();
        }
    }
}

NetResult<void> TcpStream::setLinger(bool enable, int timeoutMs) {
    struct linger lg = {};
    lg.l_onoff = enable ? 1 : 0;
    lg.l_linger = timeoutMs / 1000; // convert ms to seconds

    return mapResult(setsockopt(m_fd, SOL_SOCKET, SO_LINGER, &lg, sizeof(lg)));
}

NetResult<void> TcpStream::shutdown(ShutdownMode mode) {
    int how;
    switch (mode) {
        case ShutdownMode::Read:
            how = SHUT_RD;
            break;
        case ShutdownMode::Write:
            how = SHUT_WR;
            break;
        case ShutdownMode::Both:
            how = SHUT_RDWR;
            break;
        default:
            return Err(Error::InvalidArgument);
    }

    return mapResult(::shutdown(m_fd, how));
}

}