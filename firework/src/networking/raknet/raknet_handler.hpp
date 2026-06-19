#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "../udp_packet.hpp"
#include "../udp_server.hpp"

namespace Firework
{
    
constexpr std::uint8_t RAKNET_MAGIC[] = { 0x00, 0xff, 0xff, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 
                                          0xfd, 0xfd, 0xfd, 0xfd, 0x12, 0x34, 0x56, 0x78 };

enum class RakNetPacketType : std::uint8_t {
    UNCONNECTED_PING_1          = 0x01,
    UNCONNECTED_PING_2          = 0x02,
    UNCONNECTED_PONG            = 0x1c,
    CONNECTED_PING              = 0x00,
    CONNECTED_PONG              = 0x03,
    OPEN_CONNECTION_REQ_1       = 0x05,
    OPEN_CONNECTION_REP_1       = 0x06,
    OPEN_CONNECTION_REQ_2       = 0x07,
    OPEN_CONNECTION_REP_2       = 0x08,
    CONNECTION_REQUEST          = 0x09,
    CONNECTION_REQUEST_ACCEPTED = 0x10,
    NEW_INCOMING_CONNECTION     = 0x13,
    DISCONNECT                  = 0x15,
    INCOMPATIBLE_PROTOCOL       = 0x19,

    FRAME_SET                   = 0x80,
    ACK                         = 0xC0,
    NACK                        = 0xA0,
};

struct ServerProperties {
    std::string_view motd1;
    // Ignored by the client
    std::string_view motd2;
    std::uint32_t playerCount;
    std::uint32_t maxPlayerCount;
    // Ignored by the client
    std::string_view gameMode{};
    // Ignored by the client
    std::uint16_t gameModeID{};
    std::uint16_t portIPv4;
    std::uint16_t portIPv6;
};

class RakNetHandler {
public:
    RakNetHandler(const ServerProperties &serverProperties, std::shared_ptr<UDPServer> udpServer);

    auto update_server_properties(const ServerProperties &serverProperties) -> void;
    auto handle_packet(const UDPPacket &packet) -> void;

private:
    std::uint64_t _guid;
    
    ServerProperties    _serverProperties;
    std::string         _cachedServerIDString;
    bool                _serverIDStringDirty;

    std::shared_ptr<UDPServer> _udpServer;

    auto properties_to_string() -> const std::string &;

    auto handle_unconnected_ping(const UDPPacket &packet) -> void;
    auto handle_connection_req_1(const UDPPacket &packet) -> void;
    auto handle_connection_req_2(const UDPPacket &packet) -> void;
};

} // namespace Firework
