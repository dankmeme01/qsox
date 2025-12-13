#include <qsox/Poll.hpp>

#include <ws2tcpip.h>

namespace qsox {

NetResult<PollResult> pollOne(BaseSocket& socket, PollType poll, int timeoutMs) {
    int fd = socket.handle();

    FD_SET readSet, writeSet, errorSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);

    if (poll & PollType::Read) {
        FD_SET(fd, &readSet);
    } else if (poll & PollType::Write) {
        FD_SET(fd, &writeSet);
    }

    FD_SET(fd, &errorSet);

    timeval tv{};

    if (timeoutMs > 0) {
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
    } else if (timeoutMs == 0) {
        tv.tv_sec = 0;
        tv.tv_usec = 1;
    } else {
        // -1 means wait indefinitely
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }

    while (true) {
        int res = ::select(0,
                           poll & PollType::Read ? &readSet : nullptr,
                           poll & PollType::Write ? &writeSet : nullptr,
                           &errorSet,
                           &tv);

        if (res == 0) {
            return Ok(PollResult::Timeout);
        } else if (res == SOCKET_ERROR) {
            return Err(Error::lastOsError());
        } else {
            break;
        }
    }

    if (FD_ISSET(fd, &readSet)) {
        return Ok(PollResult::Readable); // socket is ready for reading
    } else if (FD_ISSET(fd, &writeSet)) {
        return Ok(PollResult::Writable); // socket is ready for writing
    } else if (FD_ISSET(fd, &errorSet)) {
        return Err(socket.getSocketError()); // socket has an error
    }

    // shouldn't be reachable
    return Ok(PollResult::None);
}

}