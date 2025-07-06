#pragma once

#include "Error.hpp"
#include "BaseSocket.hpp"
#include <stddef.h>

namespace qsox {

enum class PollResult {
    None,      // should not be instantiated
    Readable,  // socket is ready for reading
    Writable,  // socket is ready for writing
    Timeout,   // poll timed out
};

struct PollType {
    enum Pt : uint8_t {
        Read =  0b001,     // poll for readability
        Write = 0b010,     // poll for writability
        ReadWrite = Read | Write,
    } p;

    PollType(Pt type) : p(type) {}

    bool operator==(const PollType& other) const {
        return p == other.p;
    }

    bool operator==(Pt other) const {
        return p == other;
    }

    bool operator&(const PollType& other) const {
        return (p & other.p) != 0;
    }

    bool operator&(Pt other) const {
        return (p & other) != 0;
    }
};

// Specify timeout in milliseconds, or -1 for indefinite wait
NetResult<PollResult> pollOne(BaseSocket& socket, PollType poll, int timeoutMs);

}