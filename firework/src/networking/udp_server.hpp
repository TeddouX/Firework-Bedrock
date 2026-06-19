#pragma once
#include <cstdint>
#include <queue>

namespace Firework::Internal
{

constexpr std::size_t MAX_PACKET_SIZE = 2048ULL;

struct AddressInfo {
    std::uint16_t port;
    std::string_view ipAddr;
};

struct UDPPacket {
    AddressInfo addrInfo;
    std::uint8_t data[MAX_PACKET_SIZE];
    size_t dataSize;
};

class UDPServer {
public:
    virtual ~UDPServer() = default;

    virtual auto create_socket(std::uint32_t port) -> bool = 0;
    virtual auto start() -> void = 0;
    virtual auto stop() -> void = 0;
    virtual auto try_pop_packet(UDPPacket &outPacket) -> bool = 0;

protected:
    virtual auto receive_thread() -> void = 0;
};

} // namespace Firework::Internal

