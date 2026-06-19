#include <memory>
#include <print>

#include "networking/windows/win_udp_server.hpp"

using namespace Firework::Internal;

int main() {
    std::shared_ptr<UDPServer> serv = std::make_shared<WinUDPServer>();
    if (!serv->create_socket(8080))
        return 0;

    serv->start();

    bool running = true;
    while (running) {
        UDPPacket packet;
        while (serv->try_pop_packet(packet)) {
            std::print("Received from {}:{} -> ", packet.addrInfo.ipAddr, packet.addrInfo.port);
            for (int i = 0; i < packet.dataSize; i++)
                std::print("{:02X} ", packet.data[i]);
            std::print("; size = {}\n", packet.dataSize);
        }
    }
}
