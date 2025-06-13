#include <qsox/BaseSocket.hpp>
#include <sys/socket.h>
#include <sys/ioctl.h>

namespace qsox {

NetResult<> BaseSocket::setNonBlocking(bool nonBlocking) {
    int nb = nonBlocking ? 1 : 0;

    if (-1 == ::ioctl(m_fd, FIONBIO, &nb)) {
        return Err(Error::lastOsError());
    }

    return Ok();
}

NetResult<> BaseSocket::_setTimeout(int kind, uint32_t timeoutMs) {
    if (timeoutMs == 0) {
        return Err(Error::InvalidArgument);
    }

    struct timeval tv = {};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;

    return mapResult(setsockopt(m_fd, SOL_SOCKET, kind, &tv, sizeof(tv)));
}

}