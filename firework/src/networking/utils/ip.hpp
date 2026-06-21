#pragma once 
#include <cstdint>
#include <string>
#include <array>

namespace Firework::Networking
{
    
// Big Endian byte order
auto IPv4_string_to_bytes(const std::string &ipv4Addr) -> std::array<std::uint8_t, 4>;
// Big Endian byte order
auto IPv6_string_to_bytes(const std::string &ipv6Addr) -> std::array<std::uint8_t, 16>;

} // namespace Firework::Networking
