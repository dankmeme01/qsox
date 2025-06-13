#include <qsox/Util.hpp>

#if defined(_MSC_VER) && !defined(__clang__)
# include <stdlib.h>
# define BSWAP16(val) _byteswap_ushort(val)
# define BSWAP32(val) _byteswap_ulong(val)
# define BSWAP64(val) _byteswap_uint64(val)
#else
# define BSWAP16(val) (uint16_t)((val >> 8) | (val << 8))
# define BSWAP32(val) __builtin_bswap32(val)
# define BSWAP64(val) __builtin_bswap64(val)
#endif

namespace qsox {

template <> uint8_t _byteswapImpl<uint8_t>(uint8_t value) {
    return value;
}

template <> uint16_t _byteswapImpl<uint16_t>(uint16_t value) {
    return BSWAP16(value);
}

template <> uint32_t _byteswapImpl<uint32_t>(uint32_t value) {
    return BSWAP32(value);
}

template <> uint64_t _byteswapImpl<uint64_t>(uint64_t value) {
    return BSWAP64(value);
}

}