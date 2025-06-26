#include <qsox/Error.hpp>
#include <string.h>

namespace qsox {

Error Error::lastOsError(bool) {
    auto code = errno;

    switch (code) {
        case ETIMEDOUT:
            return Code::TimedOut;
        case ECONNREFUSED:
            return Code::ConnectionRefused;
        case EBADF:
        case EINVAL:
            return Code::InvalidArgument;
        case ENOTSOCK:
            return Code::NotASocket;
        case EACCES:
            return Code::AccessDenied;
        case EFAULT:
            return Code::InvalidPointerAddress;

#if EAGAIN != EWOULDBLOCK // EAGAIN and EWOULDBLOCK are the same on linux
        case EAGAIN:
#endif
        case EWOULDBLOCK:
            return Code::WouldBlock;
        case EINPROGRESS:
        case EALREADY:
            return Code::InProgress;
        case EMSGSIZE:
            return Code::MessageTooLong;
        case EADDRINUSE:
            return Code::AddressInUse;
        case ENETDOWN:
            return Code::NetworkDown;
        case ECONNABORTED:
            return Code::ConnectionAborted;
        case ECONNRESET:
            return Code::ConnectionReset;
        case EISCONN:
            return Code::IsConnected;
        case ENOTCONN:
            return Code::NotConnected;
        default: return fromOs(code);
    }
}

std::string Error::osMessage(int osCode) {
    return strerror(osCode);
}

}