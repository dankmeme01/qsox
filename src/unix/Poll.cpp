#include <qsox/Poll.hpp>

#include <poll.h>

namespace qsox {

NetResult<PollResult> pollOne(BaseSocket& socket, PollType poll, int timeoutMs) {
    int fd = socket.handle();

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLERR;

    if (poll & PollType::Read) {
        pfd.events |= POLLIN;
    }

    if (poll & PollType::Write) {
        pfd.events |= POLLOUT;
    }

    while (true) {
        int res = ::poll(&pfd, 1, timeoutMs);

        if (res == 0) {
            return Ok(PollResult::Timeout);
        } else if (res == -1) {
            auto error = Error::lastOsError();
            if (error.osCode() == EINTR) {
                // interrupted by a signal, retry
                continue;
            } else {
                return Err(error);
            }
        } else if ((pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) != 0) {
            return Err(socket.getSocketError());
        } else {
            break;
        }

        // TODO: this should decrease interval when interrupted
    }

    if (pfd.revents & POLLIN) {
        return Ok(PollResult::Readable); // socket is ready for reading
    } else if (pfd.revents & POLLOUT) {
        return Ok(PollResult::Writable); // socket is ready for writing
    } else if (pfd.revents & POLLERR) {
        return Err(socket.getSocketError()); // socket has an error
    }

    // shouldn't be reachable
    return Ok(PollResult::None);
}

}