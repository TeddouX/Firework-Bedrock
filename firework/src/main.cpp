#include <memory>
#include <print>
#include <numeric>

#include "networking/socket/windows/win_udp_server.hpp"
#include "networking/raknet/raknet_server.hpp"

using namespace Firework;

inline static Firework::Logger LOGGER{"Firework", "Main"};

int main() {
    std::shared_ptr<Networking::Socket::UDPServer> serv = std::make_shared<Networking::Socket::WinUDPServer>();
    if (!serv->create_socket(19132))
        return 0;
    
    serv->start();

    Networking::RakNet::ServerProperties serverProperties{};
    serverProperties.motd1 = "Test Server";
    serverProperties.playerCount = 69;
    serverProperties.maxPlayerCount = 420;
    serverProperties.gameMode = "Survival";
    serverProperties.gameModeID = 1;
    serverProperties.portIPv4 = 19132;
    serverProperties.portIPv6 = 19133;

    Networking::RakNet::Server rakNetServer{serverProperties, serv};

    bool running = true;
    while (running) {
        Networking::Socket::UDPPacket packet;
        while (serv->try_pop_packet(packet)) {
            LOGGER.info("Received from {} -> ", packet.addrInfo().to_string());
            for (int i = 0; i < packet.dataSize(); i++)
                std::print("{:02X} ", packet.data()[i]);
            std::print("; size = {}\n", packet.dataSize());
        
            rakNetServer.handle_packet(packet);
        }

        rakNetServer.update_connections();
    }
}
