#pragma once
#include <chrono>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <set>
#include <array>
#include <vector>

#include "../uint24.hpp"
#include "../address.hpp"
#include "raknet_packets.hpp"

namespace Firework::Networking::RakNet
{
    
struct Connection {
    static constexpr std::chrono::duration RETRANSMISSION_TIMEOUT = std::chrono::milliseconds(100);
    static constexpr std::chrono::duration ACK_FLUSH_INTERVAL = std::chrono::milliseconds(50);
    static constexpr std::size_t MAX_PENDING_ACKS_PACKETS = 16;
    static constexpr std::size_t MAX_ORDERING_CHANNELS = 32;

    struct OrderingChannel {
        uint24_t expectedOrderIndex{0u};
        std::map<uint24_t, Frame> outOfOrderBuffer{};
    };

    struct RetransmissionEntry {
        FrameSetPacket packet;
        std::chrono::steady_clock::time_point sentAt;
    };

    // Misc
    AddressInfo address;
    bool isFullyConnected; // Past handshake, into Game Packet phase
    std::uint16_t MTU;

    // -------------
    // Outgoing side
    // -------------
    uint24_t nextSequenceNumber{0u}; // Next sequence number to assign to an outgoing datagram
    // sequence# -> raw datagram bytes, for retransmission
    // Raw datagram bytes are cleared at each ACK received by the client
    std::unordered_map<uint24_t, RetransmissionEntry> sentFrameSetPackets{};
    
    // Frame level data
    std::uint16_t nextReliableFrameIdx{0u};
    std::uint16_t nextSequencedFrameIdx{0u};
    std::uint16_t nextCompoundID{0u};
    std::array<uint24_t, MAX_ORDERING_CHANNELS> orderingChannels{0u};

    // -------------
    // Incoming side
    // -------------
    uint24_t expectedSequenceNumber{0u};                            // Next sequence number we expect to receive
    std::chrono::steady_clock::time_point lastOutgoingACKTime{};    // Used to not flood the network with acks
    std::set<uint24_t> pendingACKsequenceNumbers;

    // These were received, but one or more packets before them have not been received
    std::set<uint24_t> receivedSequenceNumbers{};
    std::set<uint24_t> missingSequenceNumbers{};
    std::chrono::steady_clock::time_point lastReceivedTime{};   // Used for detecting timeouts
    
    // Frame level data
    std::set<uint24_t> outOfOrderReliableFrames{};
    uint24_t expectedReliableFrameIdx{0u};
    uint24_t highestSequencedFrameIdx{0u};

    std::array<OrderingChannel, MAX_ORDERING_CHANNELS> incomingOrderingChannels{};

    auto on_frame_set_sent(FrameSetPacket &frameSet) -> void;

    auto on_ack(const ACKPacket &ack) -> void;
    auto on_nack(const NACKPacket &nack) -> std::vector<FrameSetPacket>;

    // Example behaviour:
    // received 0: expectedSequenceNumber = 1
    // received 3: receivedSequenceNumber{3}, missingSequenceNumber{1, 2}
    // received 4: receivedSequenceNumber{3, 4}, missingSequenceNumber{1, 2}
    // received 1: expectedSequenceNumber = 2, receivedSequenceNumber{3, 4}, missingSequenceNumber{2}
    // received 2: expectedSequenceNumber = 5
    auto update_sequence(uint24_t seq) -> void;

    // Returns an empty vector if the frame should be skipped
    auto update_frame_level_data(const Frame &frame) -> std::vector<Frame>;
};

} // namespace Firework::Networking::RakNet
