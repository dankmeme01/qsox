#include <qsox/Error.hpp>
#include <fmt/format.h>
#include <cassert>

#ifdef _MSC_VER
# include <intrin.h>
#endif

namespace qsox {

Error Error::fromOs(int osCode) {
    return Error(Code::Other, osCode);
}

Error::Code Error::code() const {
    return m_code;
}

bool Error::isOsError() const {
    return m_code == Code::Other;
}

int Error::osCode() const {
    return m_osCode;
}

bool Error::operator==(const Error& other) const {
    return m_code == other.m_code && m_osCode == other.m_osCode;
}

bool Error::operator!=(const Error& other) const {
    return !(*this == other);
}

bool Error::operator==(Code code) const {
    return m_code == code;
}

bool Error::operator!=(Code code) const {
    return m_code != code;
}

std::string Error::message() const {
    if (isOsError()) {
        return fmt::format("{} (OS error {})", osMessage(m_osCode), m_osCode);
    }

    switch (m_code) {
        case Code::Success:
            return "Success";
        case Code::UnsupportedFamily:
            return "Unsupported address family";
        case Code::InvalidArgument:
            return "Invalid argument";
        case Code::InvalidPointerAddress:
            return "Invalid pointer address";
        case Code::IsConnected:
            return "Socket is already connected";
        case Code::NotConnected:
            return "Socket is not connected";
        case Code::NotASocket:
            return "Attempting to perform operation on a non-socket";
        case Code::MessageTooLong:
            return "Message too long";
        case Code::AccessDenied:
            return "Access denied";
        case Code::TimedOut:
            return "Operation timed out";
        case Code::ConnectionRefused:
            return "Connection refused";
        case Code::ConnectionAborted:
            return "Connection aborted";
        case Code::ConnectionReset:
            return "Connection reset by peer";
        case Code::ConnectionClosed:
            return "Connection closed by peer";
        case Code::WouldBlock:
            return "Operation would block";
        case Code::InProgress:
            return "Operation already in progress";
        case Code::AddressInUse:
            return "Address already in use";
        case Code::NetworkDown:
            return "Network is unreachable";
        case Code::AlreadyShutdown:
            return "Socket is already shutdown";
        case Code::Unimplemented:
            return "Operation is not implemented";
        case Code::Other:
            unreachable();
    }

    unreachable();
}

[[noreturn]] void unreachable() {
    assert(false && "Unreachable code reached");

#if defined(__clang__) || defined(__GNUC__)
    __builtin_unreachable();
#else
    __assume(false);
#endif
}

} // namespace qsox