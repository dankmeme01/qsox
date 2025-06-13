#include <qsox/BaseSocket.hpp>
#include <WinSock2.h>

namespace qsox {

NetResult<> BaseSocket::setNonBlocking(bool nonBlocking) {
    unsigned long nb = nonBlocking ? 1 : 0;

    if (::ioctlsocket(m_fd, FIONBIO, &nb) < 0) {
        return Err(Error::lastOsError());
    }

    return Ok();
}

NetResult<> BaseSocket::_setTimeout(int kind, uint32_t timeoutMs) {
    if (timeoutMs == 0) {
        return Err(Error::InvalidArgument);
    }

    // windows uses DWORD instead of timeval

    return mapResult(
        setsockopt(m_fd, SOL_SOCKET, kind, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs))
    );
}

}