#pragma once
#include <bit>
#include <vector>
#include <cstdint>

#include "uint24.hpp"

namespace Firework
{
    
template <typename _Ty>
concept IsIntegral = requires { std::is_integral_v<_Ty> | std::is_same_v<_Ty, uint24_t>; };

template <IsIntegral _Integral>
constexpr _Integral network_to_host(const _Integral &value) {
    if constexpr (std::is_same_v<_Integral, uint24_t>) {
        return std::endian::native == std::endian::little 
            ? value.byteswap()
            : value;    
    }

    return std::endian::native == std::endian::little 
        ? std::byteswap(value)
        : value;
} 

template <IsIntegral _Integral>
constexpr _Integral host_to_network(const _Integral &value) {
    // They are the same
    return network_to_host(value);
}

template <IsIntegral _Integral>
constexpr const std::uint8_t *get_int_bytes(const _Integral &value) {
    if constexpr (std::is_same_v<_Integral, uint24_t>)
        return value.get_bytes();
    return reinterpret_cast<const std::uint8_t *>(std::addressof(value));
}

template <IsIntegral _Integral>
constexpr _Integral int_from_bytes(const std::uint8_t *value) {
    if constexpr (std::is_same_v<_Integral, uint24_t>)
        return uint24_t{value};
    return *reinterpret_cast<const _Integral *>(value);
}

} // namespace Firework

