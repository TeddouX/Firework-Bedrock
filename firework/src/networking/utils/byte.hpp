#pragma once
#include <bit>
#include <vector>
#include <cstdint>

namespace Firework
{
    
template <typename _Integral>
    requires std::is_integral_v<_Integral>
constexpr _Integral network_to_host(const _Integral &value) {
    return std::endian::native == std::endian::little 
        ? std::byteswap(value)
        : value;
} 

template <typename _Integral>
    requires std::is_integral_v<_Integral>
constexpr _Integral host_to_network(const _Integral &value) {
    // They are the same
    return network_to_host(value);
}

template <typename _Integral>
    requires std::is_integral_v<_Integral>
constexpr const std::uint8_t *get_int_bytes(const _Integral &value) {
    return reinterpret_cast<const std::uint8_t *>(std::addressof(value));
}

template <typename _Integral>
    requires std::is_integral_v<_Integral>
constexpr const _Integral &int_from_bytes(const std::uint8_t *value) {
    return *reinterpret_cast<const _Integral *>(value);
}

} // namespace Firework

