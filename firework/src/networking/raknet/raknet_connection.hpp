#pragma once
#include <chrono>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <set>
#include <array>

#include "../utils/uint24.hpp"
#include "../address.hpp"
#include "raknet_packets.hpp"

namespace Firework::Networking
{
    
struct RakNetConnection {
    static constexpr std::chrono::duration ACK_FLUSH_INTERVAL = std::chrono::milliseconds(10);
    static constexpr std::size_t MAX_ORDERING_CHANNELS = 32;

    struct OrderingChannel {
        uint24_t expectedOrderIndex{0u};
        std::map<uint24_t, FrameSetPacket::Frame> outOfOrderBuffer{};
    };

    AddressInfo address;

    // Outgoing side
    uint24_t nextSequenceNumber{0u}; // Next sequence number to assign to an outgoing datagram
    // sequence# -> raw datagram bytes, for retransmission
    // Raw datagram bytes are cleared at each ACK received by the client
    std::unordered_map<uint24_t, std::vector<std::uint8_t>> sentDatagrams{}; 

    // Incoming side
    uint24_t expectedSequenceNumber{0u};                        // Next sequence number we expect to receive
    std::chrono::steady_clock::time_point lastACKTime{};        // Used to not flood the network with acks
    std::set<uint24_t> pendingACKsequenceNumbers;

    // These were received, but one or more packets before them have not been received
    std::set<uint24_t> receivedSequenceNumbers{};
    std::set<uint24_t> missingSequenceNumbers{};
    std::chrono::steady_clock::time_point lastReceivedTime{};   // Used for detecting timeouts
    
    // Frame level data
    std::set<uint24_t> outOfOrderReliableFrames{};
    uint24_t expectedReliableFrameIdx{0u};
    uint24_t highestSequencedFrameIdx{0u};

    std::array<OrderingChannel, MAX_ORDERING_CHANNELS> orderingChannels{};

    // Misc
    bool isFullyConnected; // Past handshake, into Game Packet phase

    // Example behaviour:
    // received 0: expectedSequenceNumber = 1
    // received 3: receivedSequenceNumber{3}, missingSequenceNumber{1, 2}
    // received 4: receivedSequenceNumber{3, 4}, missingSequenceNumber{1, 2}
    // received 1: expectedSequenceNumber = 2, receivedSequenceNumber{3, 4}, missingSequenceNumber{2}
    // received 2: expectedSequenceNumber = 5
    auto update_sequence(uint24_t seq) -> void;

    // Returns an empty vector if the frame should be skipped
    auto update_frame_level_data(const FrameSetPacket::Frame &frame) -> std::vector<FrameSetPacket::Frame>;
};

} // namespace Firework::Networking
