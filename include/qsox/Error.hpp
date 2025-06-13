#pragma once

#include <Geode/Result.hpp>

namespace qsox {
    class Error {
    public:
        typedef enum {
            Success = 0,
            UnsupportedFamily,
            InvalidArgument,
            InvalidPointerAddress,
            IsConnected,
            NotConnected,
            NotASocket,
            MessageTooLong,
            AccessDenied,
            TimedOut,
            ConnectionRefused,
            ConnectionAborted,
            ConnectionReset,
            ConnectionClosed,
            WouldBlock,
            InProgress,
            AddressInUse,
            NetworkDown,
            AlreadyShutdown,
            Other, // meaning other OS error
        } Code;

        constexpr inline Error(Code code, int osCode = 0) : m_code(code), m_osCode(osCode) {}

        static Error fromOs(int osCode);
        // `net` - whether to use WSAGetLastError()
        static Error lastOsError(bool net = true);

        Code code() const;
        bool isOsError() const;
        int osCode() const;

        bool operator==(const Error& other) const;
        bool operator!=(const Error& other) const;
        bool operator==(Code code) const;
        bool operator!=(Code code) const;

        std::string message() const;

    protected:
        Code m_code;
        int m_osCode = 0;

        static std::string osMessage(int osCode);
    };

    template <typename T = void, typename E = std::string>
    using Result = geode::Result<T, E>;

    template <typename T = void>
    using NetResult = geode::Result<T, Error>;

    using geode::Ok;
    using geode::Err;

    inline NetResult<> mapResult(int res) {
        if (res == -1) {
            return Err(Error::lastOsError());
        }

        return Ok();
    }

    [[noreturn]] void unreachable();
}

#define QSOX_MAKE_ERROR_STRUCT(name, ...) \
    class name { \
    public: \
        typedef enum {\
            __VA_ARGS__ \
        } Code; \
        constexpr inline name(Code code) : m_code(code) {} \
        constexpr inline name(const name& other) = default; \
        constexpr inline name& operator=(const name& other) = default; \
        constexpr inline Code code() const { return m_code; } \
        constexpr inline bool operator==(const name& other) const { return m_code == other.m_code; } \
        constexpr inline bool operator!=(const name& other) const { return !(*this == other); } \
        std::string_view message() const; \
    private: \
        Code m_code; \
    }