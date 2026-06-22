#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <array>

#include "../address.hpp"
#include "../udp_packet.hpp"
#include "../uint24.hpp"

namespace Firework::Networking
{
    
constexpr std::uint8_t RAKNET_MAGIC[] = { 0x00, 0xff, 0xff, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 
                                          0xfd, 0xfd, 0xfd, 0xfd, 0x12, 0x34, 0x56, 0x78 };
constexpr std::size_t RAKNET_MAGIC_SIZE = sizeof(RAKNET_MAGIC);

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

// ID: uint8
// Time: uint64 little-endian
// MAGIC
// Client GUID: uint64 little-endian
struct UnconnectedPingPacket {
    std::uint64_t time;
    std::uint64_t clientGUID;

    static constexpr std::size_t SIZE = 1 
        + sizeof(time) 
        + RAKNET_MAGIC_SIZE 
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

    auto encode() -> std::vector<std::uint8_t>;
};

// ID: uint8
// MAGIC
// Protocol version: byte
// MTU: zero padding
struct OpenConnectionRequest1Packet {
    std::uint8_t protocolVersion;
    std::uint16_t MTU;

    static constexpr std::size_t MIN_SIZE = 1
        + RAKNET_MAGIC_SIZE
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
        + RAKNET_MAGIC_SIZE
        + sizeof(serverGUID)
        + sizeof(bool)
        + sizeof(MTU); 

    auto encode() -> std::vector<std::uint8_t>;
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
    std::uint64_t clientGUI;

    static constexpr std::size_t SIZE = 1
        + RAKNET_MAGIC_SIZE
        + 7
        + sizeof(MTU)
        + sizeof(clientGUI);

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
    AddressInfo clientAddress;
    std::uint16_t MTU;
    
    static constexpr std::size_t SIZE = 1
        + RAKNET_MAGIC_SIZE
        + sizeof(serverGUID)
        + 7
        + sizeof(MTU)
        + sizeof(bool);

    auto encode() -> std::vector<std::uint8_t>;
};

class FrameSetPacket {
public:
    enum class Reliability : std::uint8_t {
                                            // Is reliable      Is ordered      Is sequenced
        Unreliable = 0,                     // False            False           False
        UnreliableSequenced = 1,            // False            True            True
        Reliable = 2,                       // True             False           False
        ReliableOrdered = 3,                // True             True            False
        ReliableSequenced = 4,              // True             True            True
        UnreliableWithAckReceipt = 5,       // False            False           False
        ReliableWithAckReceipt = 6,         // True             False           False
        ReliableOrderedWithAckReceipt = 7,  // True             True            False
    };

    static auto is_reliable(Reliability reliability) -> bool;
    static auto is_sequenced(Reliability reliability) -> bool;
    static auto is_ordered(Reliability reliability) -> bool;

    struct Frame {
        Reliability reliability;
        // Deduced from reliability
        bool isReliable;
        bool isSequenced;
        bool isOrdered;

        bool isFragmented;
        
        std::uint16_t payloadSizeBits; // In bits

        // Only if reliable
        std::optional<uint24_t> reliableFrameIndex;
        // Only if sequenced
        std::optional<uint24_t> sequencedFrameIndex;
        
        // Only if ordered
        std::optional<uint24_t> orderedFrameIndex;
        std::optional<uint8_t> orderChannel;

        // Only if isFragmented
        std::optional<std::int32_t> compoundSize;
        std::optional<std::int16_t> compoundID;
        std::optional<std::int32_t> fragmentIdx;

        std::vector<std::uint8_t> payload;
    };

    static auto from_packet(const std::vector<std::uint8_t> &data) -> std::optional<FrameSetPacket>;

    FrameSetPacket(Reliability reliability, const std::vector<const std::vector<std::uint8_t> *> &packets, size_t packetsTotalSize);

    // Returns each packet that needs to be sent in case of splitting if data is too big
    auto encode() -> std::vector<std::vector<std::uint8_t>>;

    auto frames() const -> const std::vector<Frame> & { return _frames; } 
    auto sequence_number() const -> const uint24_t & { return _sequenceNumber; } 

private:
    // Unknown what these are used for, so we will just ignore them
    // bool             _isPacketPair;
    // bool             _isContinuousSend;
    // bool             _needs_B_and_AS;

    uint24_t            _sequenceNumber;
    std::vector<Frame>  _frames;

    FrameSetPacket() = default;
};

} // namespace Firework::Networking
