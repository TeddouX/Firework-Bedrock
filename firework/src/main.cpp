#include <memory>
#include <print>
#include <numeric>

#include "networking/socket/windows/win_udp_server.hpp"
#include "networking/raknet/raknet_server.hpp"
#include "networking/bedrock_protocol/bp_decryptor.hpp"
#include "core/codec/object_codec.hpp"

using namespace Firework;

inline static Firework::Logger LOGGER{"Firework", "Main"};

class Foo {
public:
    Foo() = default;
    
    using Codec = Firework::ObjectCodec<Foo, unsigned int, Skip<float>, unsigned short>;

    unsigned int a;
    float b;
    unsigned short c;
};

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

    Foo foo{};
    foo.a = 21;
    foo.b = 54.f;
    foo.c = 50;

    auto data = Foo::Codec::encode(foo);
    for (int i = 0; i < data.size(); i++)
        std::print("0x{:02X} ", data[i]);
    std::print("\n");

    auto foo2 = Foo::Codec::decode(data);
    if (!foo2) return 0;
    std::println("{} {:.2f} {}",
        foo2->a,
        foo2->b,
        foo2->c
    );
}
