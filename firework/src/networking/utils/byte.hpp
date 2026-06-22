#pragma once
#include <bit>
#include <vector>
#include <cstdint>

#include "../uint24.hpp"
#include "../../firework.hpp"

namespace Firework::Networking
{

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

} // namespace Firework::Networking

