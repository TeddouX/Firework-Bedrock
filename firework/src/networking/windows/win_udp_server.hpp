#pragma once
#include <cstdint>
#include <queue>
#include <mutex>
#include <thread>

#include "../udp_server.hpp"

namespace Firework::Internal
{
    
using Socket = std::uint64_t;

class WinUDPServer : public UDPServer {
public:
    WinUDPServer();

    auto create_socket(std::uint32_t port) -> bool override;
    auto start() -> void override;
    auto stop() -> void override;
    auto try_pop_packet(UDPPacket &outPacket) -> bool override;

protected:
    auto receive_thread() -> void override;

private:
    Socket _receiveSocket;

    std::thread _receiveThread;
    std::atomic_bool _running;
    std::mutex _packetsMutex;
    std::queue<UDPPacket> _packetsQueue;
};

} // namespace Firework::Internal