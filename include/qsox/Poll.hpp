#pragma once

#include "Error.hpp"
#include <stddef.h>

namespace qsox {

enum class PollResult {
    None,      // should not be instantiated
    Readable,  // socket is ready for reading
    Writable,  // socket is ready for writing
    Timeout,   // poll timed out
};

enum class PollType {
    Read,      // poll for readability
    Write,     // poll for writability
    Error,     // poll for errors
};

// Specify timeout in milliseconds, or -1 for indefinite wait
// TODO: this is not properly implemented nor tested
NetResult<PollResult> pollOne(int fd, PollType poll, int timeoutMs);

}