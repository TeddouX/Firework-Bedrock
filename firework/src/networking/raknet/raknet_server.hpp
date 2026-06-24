#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>
#include <set>
#include <unordered_map>
#include <map>
#include <functional>

#include "../socket/udp_server.hpp"
#include "../uint24.hpp"
#include "raknet_connection.hpp"

namespace Firework::Networking::RakNet
{
    
// Reserved for raknet communication
constexpr inline std::uint8_t FIREWORK_RAKNET_RESERVED_CHANNEL = 0; 

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

class Server {
public:
    Server(const ServerProperties &serverProperties, std::shared_ptr<Socket::UDPServer> udpServer);

    auto update_server_properties(const ServerProperties &serverProperties) -> void;
    auto handle_packet(const Socket::UDPPacket &packet) -> void;
    auto update_connections() -> void;
    auto get_time_ms_since_start() -> std::chrono::milliseconds;
    auto on_game_packet_received(std::function<void (GamePacket &)> handler) -> void;

private:
    std::uint64_t _guid;
    
    ServerProperties    _serverProperties;
    std::string         _cachedServerIDString;
    bool                _serverIDStringDirty;

    std::chrono::steady_clock::time_point _startTime;

    std::shared_ptr<Socket::UDPServer> _udpServer;
    // ip:port -> connection, these are the connections that have sent a Connection Request 1 packet
    std::unordered_map<Address, Connection> _openConnections;

    std::function<void (GamePacket &)> _gamePacketHandler;

    auto properties_to_string() -> const std::string &;
    
    auto handle_unconnected_ping(const Address &addrInfo, const UnconnectedPingPacket &packet) -> void;
    auto handle_open_connection_req_1(const Address &addrInfo, const OpenConnectionRequest1Packet &packet) -> void;
    auto handle_open_connection_req_2(const Address &addrInfo, const OpenConnectionRequest2Packet &packet) -> void;
    auto handle_connection_request(const Address &addrInfo, const ConnectionRequestPacket &packet) -> void;
    auto handle_ack(const Address &addrInfo, const ACKPacket &packet) -> void;
    auto handle_nack(const Address &addrInfo, const NACKPacket &packet) -> void;
    auto handle_disconnect(const Address &addrInfo) -> void;
    auto handle_connected_ping(const Address &addrInfo, const ConnectedPingPacket &packet) -> void;

    auto decode_frame_set(const Socket::UDPPacket &packet) -> std::vector<Frame>;
    auto send_in_frame_set(Connection &connection, PartialFrame &partialFrame) -> bool;
    auto send_frame_sets(Connection &connection, std::vector<FrameSetPacket> &frameSets) -> bool;
    auto send_frame_set(Connection &connection, const FrameSetPacket &frameSet) -> bool;

    auto handle_packet(const Address &addrInfo, const std::vector<std::uint8_t> &packet) -> void;

    auto get_connection(const Address &addrInfo) -> Connection *;
};

} // namespace Firework::Networking::RakNet
