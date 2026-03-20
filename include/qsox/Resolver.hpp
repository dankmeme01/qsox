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
    TimedOut,
    Other,
);

template <typename T>
using Result = qsox::Result<T, Error>;

/// Resolves a single IPv4 address that belongs to the given domain name.
/// If timeout is nonzero, on supported systems (Windows and Linux with GLIBC) this function will time out after the given duration.
Result<Ipv4Address> resolveIpv4(const std::string& hostname, int timeoutMs = 0);
/// Resolves a single IPv6 address that belongs to the given domain name
/// If timeout is nonzero, on supported systems (Windows and Linux with GLIBC) this function will time out after the given duration.
Result<Ipv6Address> resolveIpv6(const std::string& hostname, int timeoutMs = 0);
/// Resolves a single IP address that belongs to the given domain name.
/// This function prefers IPv4 addresses, and will only return an IPv6 address if the IPv4 lookup fails.
/// If timeout is nonzero, on supported systems (Windows and Linux with GLIBC) this function will time out after the given duration.
/// Note that the timeout is applied separately per lookup, instead of on the whole function.
Result<IpAddress> resolve(const std::string& hostname, int timeoutMs = 0);

Error makeError(int code);

} // namespace qsox