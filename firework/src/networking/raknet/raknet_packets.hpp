#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <array>

#include "../socket/udp_packet.hpp"
#include "../address.hpp"
#include "../uint24.hpp"
#include "raknet_frame_set_packet.hpp"

namespace Firework::Networking::RakNet
{
    
constexpr std::uint8_t MAGIC[] = { 0x00, 0xff, 0xff, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 
                                          0xfd, 0xfd, 0xfd, 0xfd, 0x12, 0x34, 0x56, 0x78 };
constexpr std::size_t MAGIC_SIZE = sizeof(MAGIC);

enum class PacketType : std::uint8_t {
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

    GAME_PACKET                 = 0xFE,
};

// ID: uint8
// Time: uint64 little-endian
// MAGIC
// Client GUID: uint64 little-endian
struct UnconnectedPingPacket {
    std::uint64_t time;
    std::uint64_t clientGUID;

    static constexpr std::size_t SIZE = 1 
        + sizeof(time) 
        + MAGIC_SIZE 
        + sizeof(clientGUID);

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<UnconnectedPingPacket>;
};

// ID: uint8
// Echoed Time: uint64 little-endian
// Server GUID: uint64 little-endian
// MAGIC
// Server ID String length: uint16 little-endian
// Server ID String: string
struct UnconnectedPongPacket {
    std::int64_t echoedTime;
    std::uint64_t serverGUID;
    std::string serverIdString;

    auto encode() const -> std::vector<std::uint8_t>;
};

// ID: uint8
// MAGIC
// Protocol version: byte
// MTU: zero padding
struct OpenConnectionRequest1Packet {
    std::uint8_t protocolVersion;
    std::uint16_t MTU;

    static constexpr std::size_t MIN_SIZE = 1
        + MAGIC_SIZE
        + sizeof(protocolVersion);

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<OpenConnectionRequest1Packet>;
};

// ID: uint8
// MAGIC
// Server GUID: uint64 little-endian
// Use security: bool (false)
// Cookie: int32 (unused)
// MTU: uint16
struct OpenConnectionReply1Packet {
    std::uint64_t serverGUID;
    std::uint16_t MTU;

    static constexpr std::size_t SIZE = 1
        + MAGIC_SIZE
        + sizeof(serverGUID)
        + sizeof(bool)
        + sizeof(MTU); 

    auto encode() const -> std::vector<std::uint8_t>;
};

// ID: uint8
// MAGIC
// Cookie: int (unused)
// Has security challenge: bool (unused)
// Server address: 7 IPv4 (supported), 29 IPv6 (unsupported)
// MTU: uint16
// Client GUID: uint64
struct OpenConnectionRequest2Packet {
    std::array<std::uint8_t, 7> serverAddress{};
    std::uint16_t MTU;
    std::uint64_t clientGUID;

    static constexpr std::size_t SIZE = 1
        + MAGIC_SIZE
        + 7
        + sizeof(MTU)
        + sizeof(clientGUID);

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<OpenConnectionRequest2Packet>;
};

// ID: uint8
// MAGIC
// Server GUID: uint64
// Client Address: 7 IPv4 (supported), 29 IPv6 (unsupported)
// MTU: uint16
// Encryption enabled: boolean (false)
struct OpenConnectionReply2Packet {
    std::uint64_t serverGUID;
    Address clientAddress;
    std::uint16_t MTU;
    
    static constexpr std::size_t SIZE = 1
        + MAGIC_SIZE
        + sizeof(serverGUID)
        + 7
        + sizeof(MTU)
        + sizeof(bool);

    auto encode() const -> std::vector<std::uint8_t>;
};

// ID: uint8
// Client GUID: uint64
// Send Time: uint64
// Use Security: bool (unused, false)
struct ConnectionRequestPacket {
    std::uint64_t clientGUID;
    std::uint64_t sendTime;

    static constexpr std::size_t SIZE = 1
        + sizeof(clientGUID)
        + sizeof(sendTime)
        + sizeof(bool);

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<ConnectionRequestPacket>;
};

// ID: uint8
// Client Address: 7 IPv4 (supported), 29 IPv6 (unsupported)
// System index: uint16 (unused)
// Internal IDs: 10x addresses (total 70 bytes)
// Connection request time: uint64
// Send time: uint64
struct ConnectionRequestAcceptedPacket {
    Address clientAddress;
    std::uint64_t connectionRequestTime;
    std::uint64_t sendTime;

    static constexpr std::size_t SIZE = 1
        + 7
        + sizeof(std::uint16_t)
        + 10 * 7
        + sizeof(connectionRequestTime)
        + sizeof(sendTime);

    auto encode() const -> std::vector<std::uint8_t>;
};

struct Record {
    bool isSingle;

    // If isSingle == false
    std::optional<uint24_t> sequenceNumber;

    // If isSingle == true
    std::optional<uint24_t> startSequenceNumber;
    std::optional<uint24_t> endSequenceNumber;

    Record() = default;
    Record(std::uint32_t rangeStart, std::uint32_t rangeEnd);
};

// ID: uint8
// Record count: uint16
// Records: Record[]
struct ACKPacket {
    std::vector<Record> records;

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<ACKPacket>;
    auto encode() const -> std::vector<std::uint8_t>;
};

// ID: uint8
// Record count: uint16
// Records: Record[]
struct NACKPacket {
    std::vector<Record> records;

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<NACKPacket>;
    auto encode() const -> std::vector<std::uint8_t>;
};

// ID: uint8
// Time: uint64
struct ConnectedPingPacket {
    std::uint64_t time;

    static constexpr std::size_t SIZE = 1
        + sizeof(time);

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<ConnectedPingPacket>;
};

// ID: uint8
// Ping Time: uint64
// Pong Time: uint64
struct ConnectedPongPacket {
    std::uint64_t pingTime;
    std::uint64_t pongTime;

    static constexpr std::size_t SIZE = 1
        + sizeof(pingTime)
        + sizeof(pongTime);

    auto encode() const -> std::vector<std::uint8_t>;
};

// ID: uint8
// data: uint8 (remaining size of the packet)
struct GamePacket {
    std::vector<std::uint8_t> data;
    Address addr;

    static auto from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<GamePacket>;
};

} // namespace Firework::Networking::RakNet
