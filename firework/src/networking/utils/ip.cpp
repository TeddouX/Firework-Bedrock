#include "ip.hpp"

#include "byte.hpp"
#include "../networking.hpp"
#include "../../firework.hpp"

#ifdef FIREWORK_WINDOWS
    #include <ws2tcpip.h>
#elifdef FIREWORK_LINUX
    #error "Linux not implemented at this time"
#endif

namespace Firework::Networking
{
 
auto IPv4_string_to_bytes(const std::string &ipv4Addr) -> std::array<std::uint8_t, 4> {
#ifdef FIREWORK_WINDOWS
    std::array<std::uint8_t, 4> bytes{};

    int result = ::inet_pton(AF_INET, ipv4Addr.c_str(), bytes.data());
    if (result == -1) {
        LOGGER.error("Error encountered while trying to decode IPv4 address: {}", ::WSAGetLastError());
        return {};
    }

    if (result == 0) {
        LOGGER.error("Invalid format for IPv4 address: {}", ipv4Addr);
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
    
    int result = ::inet_pton(AF_INET6, ipv6Addr.c_str(), bytes.data());
    if (result == -1) {
        LOGGER.error("Error encountered while trying to decode IPv6 address: {}", ::WSAGetLastError());
        return {};
    }

    if (result == 0) {
        LOGGER.error("Invalid format for IPv6 address: {}", ipv6Addr);
        return {};
    }
    
    return bytes;
#elifdef FIREWORK_LINUX
    return {};
#endif
}

} // namespace Firework::Networking
