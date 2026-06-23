#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#include "../uint24.hpp"

namespace Firework::Networking::RakNet
{

struct Connection;

struct Frame {
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

    static constexpr std::size_t COMMON_HEADER_SIZE     = sizeof(Reliability) + sizeof(std::uint16_t);
    static constexpr std::size_t RELIABLE_HEADER_SIZE   = sizeof(uint24_t);
    static constexpr std::size_t ORDERED_HEADER_SIZE    = sizeof(uint24_t) + sizeof(std::uint8_t);
    static constexpr std::size_t SEQUENCED_HEADER_SIZE  = sizeof(uint24_t) + ORDERED_HEADER_SIZE;
    static constexpr std::size_t FRAGMENT_HEADER_SIZE   = sizeof(std::uint32_t) + sizeof(std::uint16_t) + sizeof(std::uint32_t);

    Reliability reliability;
    // Deduced from reliability
    bool isReliable;
    bool isSequenced;
    bool isOrdered;

    bool isFragmented;
    
    std::uint16_t payloadSizeBits;

    // Only if reliable
    std::optional<uint24_t> reliableFrameIndex;
    // Only if sequenced
    std::optional<uint24_t> sequencedFrameIndex;
    
    // Only if ordered
    std::optional<uint24_t> orderedFrameIndex;
    std::optional<std::uint8_t> orderChannel;

    // Only if isFragmented
    std::optional<std::uint32_t> compoundSize;
    std::optional<std::uint16_t> compoundID;
    std::optional<std::uint32_t> fragmentIdx;

    std::vector<std::uint8_t> payload;
};

struct PartialFrame {
    Frame::Reliability reliability;

    // Only if ordered
    // 0 is reserved, but default so be careful ;)
    std::uint8_t orderChannel{0u};

    std::vector<std::uint8_t> payload;
};

class FrameSetPacket {
public:
    static auto is_reliable(Frame::Reliability reliability) -> bool;
    static auto is_sequenced(Frame::Reliability reliability) -> bool;
    static auto is_ordered(Frame::Reliability reliability) -> bool;

    static auto from_packet(const std::vector<std::uint8_t> &data) -> std::optional<FrameSetPacket>;

    // Do not access the frames or data contained in them after this method was called
    static auto from_partial_frames(std::vector<PartialFrame> &frames, Connection &connection) -> std::vector<FrameSetPacket>;
    
    auto encode() const -> std::vector<std::uint8_t>;
    
    auto frames() const -> const std::vector<Frame> & { return _frames; } 
    auto sequence_number() const -> const uint24_t & { return _sequenceNumber; } 
    
private:
    uint24_t            _sequenceNumber;
    std::vector<Frame>  _frames;
    
    FrameSetPacket() = default;
};

} // namespace Firework::Networking::RakNet
