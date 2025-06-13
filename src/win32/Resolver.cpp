#include <qsox/Resolver.hpp>
#include <ws2tcpip.h>

namespace qsox::resolver {

Error makeError(int code) {
    switch (code) {
        case EAI_AGAIN:
            return Error::TemporaryFailure;
        case EAI_FAIL:
            return Error::PermanentFailure;
        case EAI_NONAME:
            return Error::UnknownHost;
        case EAI_SERVICE:
            return Error::ServiceNotFound;
        case EAI_SOCKTYPE:
            return Error::SocketNotSupported;
        case EAI_MEMORY:
            return Error::OutOfMemory;
        default:
            return Error::Other;
    }
}

} // namespace qsox::resolver
