#pragma once

#include <charconv>
#include <string_view>
#include <optional>

template <typename T>
inline std::optional<T> parseNum(std::string_view v, int base = 10, bool allowTrailingData = false) {
    T out;

    auto start = v.data();
    auto end = v.data() + v.size();
    auto result = std::from_chars(start, end, out, base);

    if (result.ec != std::errc()) {
        return std::nullopt;
    }

    if (!allowTrailingData && result.ptr != end) {
        return std::nullopt;
    }

    return out;
}
