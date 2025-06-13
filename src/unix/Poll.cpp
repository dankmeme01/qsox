#include <qsox/Poll.hpp>

#include <poll.h>

namespace qsox {

NetResult<PollResult> pollOne(int fd, PollType poll, int timeoutMs) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = 0;

    switch (poll) {
        case PollType::Read:
            pfd.events = POLLIN;
            break;
        case PollType::Write:
            pfd.events = POLLOUT;
            break;
        case PollType::Error:
            pfd.events = POLLERR;
            break;
    }

    if (timeoutMs == 0) {
        timeoutMs = 1;
    }

    while (true) {
        // TODO: windows uses select
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
        } else if ((pfd.revents & (POLLHUP | POLLERR)) != 0) {
            // TODO: idk if this is correct
            return Err(Error::lastOsError());
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
        // TODO: maybe get SO_ERROR or smth
        return Err(Error::lastOsError()); // socket has an error
    }

    // TODO: uhh idk if reachable
    return Ok(PollResult::None);
}

}