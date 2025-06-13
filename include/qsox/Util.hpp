#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Big endian hosts? who cares!
// #define QSOX_BIG_ENDIAN 1

namespace qsox {

template <typename>
constexpr bool AlwaysFalse = false;

template <typename To, typename From>
To bitCast(const From& from) {
    static_assert(sizeof(To) == sizeof(From), "bitCast requires types of the same size");

    To to;
    memcpy(&to, &from, sizeof(To));
    return to;
}

// Byteswap implementation, valid only for uint[8|16|32|64]_t types.
template <typename T>
T _byteswapImpl(T value);

// Declare specializations (definitions in Util.cpp)
template <> uint8_t _byteswapImpl<uint8_t>(uint8_t value);
template <> uint16_t _byteswapImpl<uint16_t>(uint16_t value);
template <> uint32_t _byteswapImpl<uint32_t>(uint32_t value);
template <> uint64_t _byteswapImpl<uint64_t>(uint64_t value);

// Reverses the byte order of a value
template <typename T>
T byteswap(T value) {
    auto roundTrip = []<typename I>(T value) -> T {
        return bitCast<T>(_byteswapImpl<I>(bitCast<I>(value)));
    };

    if constexpr (sizeof(T) == 1) {
        return roundTrip.template operator()<uint8_t>(value);
    } else if constexpr (sizeof(T) == 2) {
        return roundTrip.template operator()<uint16_t>(value);
    } else if constexpr (sizeof(T) == 4) {
        return roundTrip.template operator()<uint32_t>(value);
    } else if constexpr (sizeof(T) == 8) {
        return roundTrip.template operator()<uint64_t>(value);
    } else {
        static_assert(AlwaysFalse<T>, "Unsupported type for byteswap");
    }
}

// Byteswaps value but only on little-endian hosts
template <typename T>
T byteswapLittle(T value) {
#ifdef QSOX_BIG_ENDIAN
    return value;
#else
    return byteswap<T>(value);
#endif
}

// Converts T from host byte order to big-endian byte order
template <typename T>
T toBigEndian(T value) {
    return byteswapLittle<T>(value);
}

// Converts T from big-endian byte order to host byte order
template <typename T>
T fromBigEndian(T value) {
    return byteswapLittle<T>(value);
}

// bool systemSupportsIp(bool v6);

}
