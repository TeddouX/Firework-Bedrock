#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>
#include <set>
#include <unordered_map>

#include "../udp_packet.hpp"
#include "../udp_server.hpp"

namespace Firework::RakNet
{
    
using uint24_t = std::uint32_t;

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

struct RakNetConnection {
    AddressInfo address;

    // Outgoing side
    std::uint32_t nextSequenceNumber{0}; // Next sequence number to assign to an outgoing datagram
    // sequence# -> raw datagram bytes, for retransmission
    // Raw datagram bytes are cleared at each ACK received by the client
    std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> sentDatagrams{}; 
    std::chrono::steady_clock::time_point lastSendTime{}; // For retransmition of timing/timeouts

    // Incoming side
    std::uint32_t expectedSequenceNumber{0};                  // Next sequence number you expect to receive
    std::set<uint32_t> receivedSequenceNumbers{};             // Track what's arrived (for building ACK ranges)
    std::set<uint32_t> missingSequenceNumbers{};              // Gaps detected -> need to NACK these
    std::chrono::steady_clock::time_point lastReceivedTime{}; // Used for detecting timeouts
    
    bool isFullyConnected; // Past handshake, into Game Packet phase
};

class RakNetServer {
public:
    RakNetServer(const ServerProperties &serverProperties, std::shared_ptr<UDPServer> udpServer);

    auto update_server_properties(const ServerProperties &serverProperties) -> void;
    auto handle_packet(const UDPPacket &packet) -> void;

private:
    std::uint64_t _guid;
    
    ServerProperties    _serverProperties;
    std::string         _cachedServerIDString;
    bool                _serverIDStringDirty;

    std::shared_ptr<UDPServer> _udpServer;
    // ip:port -> connection, these are the connections that have sent a Connection Request 1 packet
    std::unordered_map<std::string, RakNetConnection> _openConnections;

    auto properties_to_string() -> const std::string &;

    auto handle_unconnected_ping(const UDPPacket &packet) -> void;
    auto handle_connection_req_1(const UDPPacket &packet) -> void;
    auto handle_connection_req_2(const UDPPacket &packet) -> void;
    auto decode_frame_set(const UDPPacket &packet) -> std::vector<uint8_t>;
};

} // namespace Firework::RakNet
