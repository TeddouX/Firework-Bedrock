#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace Firework
{
    
constexpr std::size_t MAX_PACKET_SIZE = 1600ULL;

struct AddressInfo {
    std::uint16_t port;
    std::string ipAddr;

    constexpr std::string to_string() const {
        return ipAddr + ":" + std::to_string(port);
    }
};

struct UDPPacket {
    AddressInfo addrInfo;
    std::uint8_t data[MAX_PACKET_SIZE];
    size_t dataSize;
};

} // namespace Firework
