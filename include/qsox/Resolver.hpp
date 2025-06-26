#pragma once

// Simple DNS resolution primitives
// Only provides A/AAAA record resolution

#include "Error.hpp"
#include "IpAddress.hpp"
#include "SocketAddress.hpp"

namespace qsox::resolver {

QSOX_MAKE_ERROR_STRUCT(Error,
    Success,
    AddrFamily,
    TemporaryFailure,
    PermanentFailure,
    OutOfMemory,
    NoData,
    UnknownHost,
    ServiceNotFound,
    SocketNotSupported,
    Other,
);

template <typename T>
using Result = qsox::Result<T, Error>;

Result<Ipv4Address> resolveIpv4(const std::string& hostname);
Result<Ipv6Address> resolveIpv6(const std::string& hostname);
Result<IpAddress> resolve(const std::string& hostname);

Error makeError(int code);

} // namespace qsox