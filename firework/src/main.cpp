#include <memory>
#include <print>

#include "networking/windows/win_udp_server.hpp"
#include "networking/raknet/raknet_server.hpp"

using namespace Firework;

int main() {
    std::shared_ptr<UDPServer> serv = std::make_shared<WinUDPServer>();
    if (!serv->create_socket(19132))
        return 0;

    serv->start();

    RakNet::ServerProperties serverProperties{};
    serverProperties.motd1 = "Test Server";
    serverProperties.playerCount = 69;
    serverProperties.maxPlayerCount = 420;
    serverProperties.gameMode = "Survival";
    serverProperties.gameModeID = 1;
    serverProperties.portIPv4 = 19132;
    serverProperties.portIPv6 = 19133;

    RakNet::RakNetServer rakNetServer{serverProperties, serv};

    bool running = true;
    while (running) {
        UDPPacket packet;
        while (serv->try_pop_packet(packet)) {
            std::print("Received from {} -> ", packet.addrInfo().to_string());
            for (int i = 0; i < packet.dataSize(); i++)
                std::print("{:02X} ", packet.data()[i]);
            std::print("; size = {}\n", packet.dataSize());
        
            rakNetServer.handle_packet(packet);
        }
    }
}
