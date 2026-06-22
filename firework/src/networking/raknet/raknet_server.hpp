#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>
#include <set>
#include <unordered_map>
#include <map>

#include "../udp_packet.hpp"
#include "../udp_server.hpp"
#include "../uint24.hpp"
#include "raknet_connection.hpp"

namespace Firework::Networking
{
    
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

class RakNetServer {
public:
    RakNetServer(const ServerProperties &serverProperties, std::shared_ptr<UDPServer> udpServer);

    auto update_server_properties(const ServerProperties &serverProperties) -> void;
    auto handle_packet(const UDPPacket &packet) -> void;
    auto update_connections() -> void;

private:
    std::uint64_t _guid;
    
    ServerProperties    _serverProperties;
    std::string         _cachedServerIDString;
    bool                _serverIDStringDirty;

    std::shared_ptr<UDPServer> _udpServer;
    // ip:port -> connection, these are the connections that have sent a Connection Request 1 packet
    std::unordered_map<AddressInfo, RakNetConnection> _openConnections;

    auto properties_to_string() -> const std::string &;

    auto handle_unconnected_ping(const AddressInfo &addrInfo, const UnconnectedPingPacket &packet) -> void;
    auto handle_open_connection_req_1(const AddressInfo &addrInfo, const OpenConnectionRequest1Packet &packet) -> void;
    auto handle_open_connection_req_2(const AddressInfo &addrInfo, const OpenConnectionRequest2Packet &packet) -> void;
    auto handle_connection_request(const AddressInfo &addrInfo, const std::vector<std::uint8_t> &packet) -> void;
    auto decode_frame_set(const UDPPacket &packet) -> std::vector<FrameSetPacket::Frame>;
    auto handle_packet(const AddressInfo &addrInfo, const std::vector<std::uint8_t> &packet) -> void;
};

} // namespace Firework::Networking
