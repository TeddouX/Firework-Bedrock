#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

#include "../address.hpp"
#include "../udp_packet.hpp"
#include "../utils/uint24.hpp"

namespace Firework::Networking
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

struct UnconnectedPongPacket {
    std::uint64_t echoedTime;
    std::uint64_t serverGUID;
    std::string serverIdString;

    auto encode() -> std::vector<std::uint8_t>;
};

struct ConnectionReply1Packet {
    std::uint64_t serverGUID;
    bool useSecurity;
    std::int32_t securityCookie;
    std::uint16_t MTU;

    auto encode() -> std::vector<std::uint8_t>;
};

struct ConnectionReply2Packet {
    std::uint64_t serverGUID;
    AddressInfo clientAddress;
    std::uint16_t MTU;
    bool useSecurity;
    
    auto encode() -> std::vector<std::uint8_t>;
};

// Frame Set
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

    static auto from_packet(const UDPPacket &packet) -> std::optional<FrameSetPacket>;

    FrameSetPacket(Reliability reliability, std::vector<const uint8_t *> packets, size_t packetsTotalSize);

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
