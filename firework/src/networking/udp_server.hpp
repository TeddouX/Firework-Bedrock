#pragma once
#include <cstdint>
#include <queue>

#include "udp_packet.hpp"

namespace Firework::Networking
{

class UDPServer {
public:
    virtual ~UDPServer() = default;

    virtual auto create_socket(std::uint32_t port) -> bool = 0;
    virtual auto start() -> void = 0;
    virtual auto stop() -> void = 0;
    virtual auto try_pop_packet(UDPPacket &outPacket) -> bool = 0;
    virtual auto send(const std::vector<std::uint8_t> &data, const AddressInfo &addrInfo) -> bool = 0;
    virtual auto send_all(const std::vector<std::vector<std::uint8_t>> &data, const AddressInfo &addrInfo) -> bool = 0;

protected:
    virtual auto receive_thread() -> void = 0;
};

} // namespace Firework::Networking

