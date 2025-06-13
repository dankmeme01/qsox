#include <qsox/Error.hpp>
#include <algorithm>
#include <Windows.h>

namespace qsox {

Error Error::lastOsError(bool wsa) {
    // TODO: map into other types than Other
    auto code = wsa ? WSAGetLastError() : GetLastError();

    switch (code) {
        case WSAETIMEDOUT:
            return Code::TimedOut;
        case WSAECONNREFUSED:
            return Code::ConnectionRefused;
        case WSAEBADF:
        case WSAEINVAL:
            return Code::InvalidArgument;
        case WSAENOTSOCK:
            return Code::NotASocket;
        case WSAEACCES:
            return Code::AccessDenied;
        case WSAEFAULT:
            return Code::InvalidPointerAddress;
        case WSAEWOULDBLOCK:
            return Code::WouldBlock;
        case WSAEINPROGRESS:
        case WSAEALREADY:
            return Code::InProgress;
        case WSAEMSGSIZE:
            return Code::MessageTooLong;
        case WSAEADDRINUSE:
            return Code::AddressInUse;
        case WSAENETDOWN:
            return Code::NetworkDown;
        case WSAECONNABORTED:
            return Code::ConnectionAborted;
        case WSAECONNRESET:
            return Code::ConnectionReset;
        case WSAEISCONN:
            return Code::IsConnected;
        case WSAENOTCONN:
            return Code::NotConnected;
        case WSAESHUTDOWN:
            return Code::AlreadyShutdown;
        default: return fromOs(code);
    }
}

std::string Error::osMessage(int osCode) {
    char buf[512];

    auto result = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, osCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        buf, sizeof(buf), nullptr
    );

    if (result == 0) {
        return "Unknown error";
    }

    std::string msg(buf, buf + result);

    // the string sometimes includes a crlf, strip it, also remove unprintable chars
    msg.erase(std::find_if(msg.rbegin(), msg.rend(), [](unsigned char ch) {
        return ch != '\r' && ch != '\n' && ch < 127;
    }).base(), msg.end());

    return msg;
}

}