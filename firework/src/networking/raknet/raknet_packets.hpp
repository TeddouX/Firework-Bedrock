#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../address.hpp"
#include "../udp_packet.hpp"
#include "../utils/uint24.hpp"

namespace Firework::RakNet
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
    enum class Reliability {
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

    struct Frame {
        Reliability reliability;
        bool isFragmented;
        std::uint16_t bufferSize; // In bits
        
        // Only if ordered
        uint24_t reliableFrameIndex{0u};
        uint24_t sequencedFrameIndex{0u};
        
        // Only if isFragmented
        std::int32_t compoundSize;
        std::int16_t compoundID;
        std::int32_t fragmentIdx;
    };

    static auto from_packet(const UDPPacket &packet) -> FrameSetPacket;

    FrameSetPacket(Reliability reliability, std::vector<const uint8_t *> packets, size_t packetsTotalSize);

    // Returns each packet that needs to be sent in case of splitting if data is too big
    auto encode() -> std::vector<std::vector<std::uint8_t>>;

private:
    // Unknown what these are used for, so we will just ignore them
    // bool        _isPacketPair;
    // bool        _isContinuousSend;
    // bool        _needs_B_and_AS;
    
    uint24_t    _sequenceNumber;

    std::vector<Frame> _frames;
};

} // namespace Firework::RakNet
