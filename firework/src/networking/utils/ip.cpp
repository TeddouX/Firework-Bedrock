#include "ip.hpp"

#include "byte.hpp"
#include "../../firework.hpp"

#ifdef FIREWORK_WINDOWS
    #include <ws2tcpip.h>
#elifdef FIREWORK_LINUX
    #error "Linux not implemented at this time"
#endif

namespace Firework
{
 
auto IPv4_string_to_bytes(const std::string &ipv4Addr) -> std::array<std::uint8_t, 4> {
#ifdef FIREWORK_WINDOWS
    std::array<std::uint8_t, 4> bytes{};
    
    // TODO: log errors

    int result = ::inet_pton(AF_INET, ipv4Addr.c_str(), bytes.data()) == SOCKET_ERROR;
    if (result == SOCKET_ERROR) {
        // Other error
        return {};
    }

    if (result == 0) {
        // Invalid format
        return {};
    }
    
    return bytes;
#elifdef FIREWORK_LINUX
    return {};
#endif
}

auto IPv6_string_to_bytes(const std::string &ipv6Addr) -> std::array<std::uint8_t, 16> {
#ifdef FIREWORK_WINDOWS
    std::array<std::uint8_t, 16> bytes{};
    
    // TODO: log errors
    
    int result = ::inet_pton(AF_INET6, ipv6Addr.c_str(), bytes.data());
    if (result == SOCKET_ERROR) {
        // Other error
        return {};
    }

    if (result == 0) {
        // Invalid format
        return {};
    }
    
    return bytes;
#elifdef FIREWORK_LINUX
    return {};
#endif
}

} // namespace Firework
