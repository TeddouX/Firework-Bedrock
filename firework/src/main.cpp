#include <memory>
#include <print>
#include <numeric>

#include "networking/socket/windows/win_udp_server.hpp"
#include "networking/raknet/raknet_server.hpp"
#include "networking/bedrock_protocol/packet/all_packets.hpp"
#include "networking/bedrock_protocol/packet/packet_registry.hpp"

using namespace Firework;

inline static Firework::Logger LOGGER{"Firework", "Main"};

int main() {
    // std::shared_ptr<Networking::Socket::UDPServer> serv = std::make_shared<Networking::Socket::WinUDPServer>();
    // if (!serv->create_socket(19132))
    //     return 0;
    
    // serv->start();

    // Networking::RakNet::ServerProperties serverProperties{};
    // serverProperties.motd1 = "Test Server";
    // serverProperties.playerCount = 69;
    // serverProperties.maxPlayerCount = 420;
    // serverProperties.gameMode = "Survival";
    // serverProperties.gameModeID = 1;
    // serverProperties.portIPv4 = 19132;
    // serverProperties.portIPv6 = 19133;

    // Networking::RakNet::Server rakNetServer{serverProperties, serv};

    // rakNetServer.on_game_packet_received([](Networking::RakNet::GamePacket &gamePacket) -> void {
    //     std::vector<std::uint8_t> compressedPayload{std::move(gamePacket.data)};
    //     std::vector<std::uint8_t> payload = Networking::BP::zlib_decompress_packet(compressedPayload);

    //     LOGGER.info("Received bedrock protocol packet from {} -> ", gamePacket.addr.to_string());
    //     for (int i = 0; i < payload.size(); i++)
    //         std::print("{:02X} ", payload[i]);
    //     std::print("; size = {}\n", payload.size());
    // });

    // bool running = true;
    // while (running) {
    //     Networking::Socket::UDPPacket packet;
    //     while (serv->try_pop_packet(packet)) {
    //         LOGGER.info("Received from {} -> ", packet.addrInfo().to_string());
    //         for (int i = 0; i < packet.dataSize(); i++)
    //             std::print("{:02X} ", packet.data()[i]);
    //         std::print("; size = {}\n", packet.dataSize());
        
    //         rakNetServer.handle_packet(packet);
    //     }

    //     rakNetServer.update_connections();
    // }

    BinaryWriter writer{10};
    auto packet = std::make_unique<Networking::BP::CS_RequestNetworkSettings>();
    packet->protocolVersion = 0x123456;
    Networking::BP::PacketRegistry::encode_packet(std::move(packet), writer);
 
    for (const auto &byte : writer.get_data())
        std::print("{:02X} ", byte);
    std::print("\n");

    BinaryReader reader{writer.get_data()};
    std::unique_ptr<Firework::Networking::BP::IBedrockPacket> bla = Networking::BP::PacketRegistry::decode_packet(
        Networking::BP::PacketType::REQUEST_NETWORK_SETTINGS, 
        reader
    );

    bla->get_type();

    std::print("{:02X}", dynamic_cast<Networking::BP::CS_RequestNetworkSettings *>(bla.get())->protocolVersion);
}
