#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>
#include <set>
#include <unordered_map>

#include "../udp_packet.hpp"
#include "../udp_server.hpp"
#include "../utils/uint24.hpp"

namespace Firework::RakNet
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

struct RakNetConnection {
    AddressInfo address;

    // Outgoing side
    uint24_t nextSequenceNumber{0u}; // Next sequence number to assign to an outgoing datagram
    // sequence# -> raw datagram bytes, for retransmission
    // Raw datagram bytes are cleared at each ACK received by the client
    std::unordered_map<uint24_t, std::vector<std::uint8_t>> sentDatagrams{}; 
    std::chrono::steady_clock::time_point lastSendTime{}; // For retransmition of timing/timeouts

    /*
    
    C seq: 0 -> S -> expectedSequenceNumber: 1
    S seq: 0 -> C 
    // Missing seq 1
    C -> S seq: 2 -> seq != expectedSequenceNumber
        => missingSequenceNumbers.append(i for i in range(seq - expectedSequenceNumber))
            & connectionDirty = true;
        => update_connections() -> NACK(1)
    C seq 1 -> S
    
    If packet is UnreliableSequenced and is received out of order, ignore it
    If a packet is UnreliableSequenced, and it is NACKed, don't resend it

    */

    // Incoming side
    uint24_t expectedSequenceNumber{0u};                        // Next sequence number you expect to receive
    uint24_t lastACKEDsequenceNumber{0u};                       // Used to build ACK ranges, and detect missing packets
    std::set<uint24_t> missingSequenceNumbers{};                // Gaps detected -> need to NACK these
    std::chrono::steady_clock::time_point lastReceivedTime{};   // Used for detecting timeouts
    
    // Misc
    bool isFullyConnected; // Past handshake, into Game Packet phase
    bool connectionDirty;  // Needs updating, set to true if datagrams were received
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
    std::unordered_map<std::string, RakNetConnection> _openConnections;

    auto properties_to_string() -> const std::string &;

    auto handle_unconnected_ping(const UDPPacket &packet) -> void;
    auto handle_connection_req_1(const UDPPacket &packet) -> void;
    auto handle_connection_req_2(const UDPPacket &packet) -> void;
    auto decode_frame_set(const UDPPacket &packet) -> std::vector<uint8_t>;
};

} // namespace Firework::RakNet
