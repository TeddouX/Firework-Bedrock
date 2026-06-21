#include "raknet_connection.hpp"

namespace Firework::Networking
{
    
auto RakNetConnection::update_sequence(uint24_t seq) -> void {
    missingSequenceNumbers.erase(seq);
    pendingACKsequenceNumbers.insert(seq);

    if (seq == expectedSequenceNumber) {
        expectedSequenceNumber++;

        // Erase all previously received sequence numbers
        while (receivedSequenceNumbers.contains(expectedSequenceNumber)) {
            receivedSequenceNumbers.erase(expectedSequenceNumber);
            missingSequenceNumbers.erase(expectedSequenceNumber);
            expectedSequenceNumber++;
        }
    }
    // We are missing a packet
    else if (seq.is_newer(expectedSequenceNumber)) {
        for (uint24_t i = expectedSequenceNumber; i < seq; i++) {
            if (!receivedSequenceNumbers.contains(i))
                missingSequenceNumbers.insert(i);
        }

        receivedSequenceNumbers.insert(seq);
    }
}

auto RakNetConnection::update_frame_level_data(const FrameSetPacket::Frame &frame) -> std::vector<FrameSetPacket::Frame> {
    std::vector<FrameSetPacket::Frame> frames;

    if (frame.isReliable && frame.reliableFrameIndex) {
        uint24_t idx = *frame.reliableFrameIndex;

        // old/duplicate
        if (expectedReliableFrameIdx.is_newer(idx))
            return {};

        
        if (idx == expectedReliableFrameIdx) {
            expectedReliableFrameIdx++;
            
            // Remove any buffered reliable frame indices as they are now contiguous
            // but avoid deleting out of order packets that should have arrived after this one
            while (outOfOrderReliableFrames.erase(expectedReliableFrameIdx))
                expectedReliableFrameIdx++;
        }
        else {
            if (!outOfOrderReliableFrames.insert(idx).second)
                return {};
        }
    }

    if (frame.isSequenced && frame.sequencedFrameIndex) {
        uint24_t idx = *frame.sequencedFrameIndex;

        // Discard out of date packages
        if (highestSequencedFrameIdx.is_newer(idx))
            return {};

        highestSequencedFrameIdx = idx;
    }

    if (frame.isOrdered && frame.orderedFrameIndex && frame.orderChannel) {
        uint24_t idx = *frame.orderedFrameIndex;
        uint24_t channel = *frame.orderChannel;
        if (channel >= static_cast<std::uint32_t>(MAX_ORDERING_CHANNELS))
            return {};

        OrderingChannel &orderingChannel = orderingChannels[static_cast<std::uint32_t>(channel)];
        if (idx == orderingChannel.expectedOrderIndex) {
            frames.push_back(frame);

            orderingChannel.expectedOrderIndex++;

            while (true) {
                auto oldFrame = orderingChannel.outOfOrderBuffer
                    .extract(orderingChannel.expectedOrderIndex);

                if (oldFrame.empty())
                    break;

                frames.push_back(std::move(oldFrame.mapped()));
                orderingChannel.expectedOrderIndex++;
            }
        }
        else {
            // old/duplicate
            if (!idx.is_newer(orderingChannel.expectedOrderIndex))
                return {};
            
            if (!orderingChannel.outOfOrderBuffer.contains(idx))
                orderingChannel.outOfOrderBuffer.emplace(idx, frame);
            return {};
        }
    }
    else
        frames.push_back(frame);

    return frames;
}

} // namespace Firework::Networking
