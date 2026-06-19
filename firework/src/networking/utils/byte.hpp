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

} // namespace Firework

