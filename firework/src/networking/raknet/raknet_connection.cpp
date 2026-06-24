#include "raknet_connection.hpp"

namespace Firework::Networking::RakNet
{
    
auto Connection::update_sequence(uint24_t seq) -> void {
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

auto Connection::update_frame_level_data(const Frame &frame) -> std::vector<Frame> {
    std::vector<Frame> frames;

    // TODO: handle split frames

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

        OrderingChannel &orderingChannel = incomingOrderingChannels[static_cast<std::uint32_t>(channel)];
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

auto Connection::on_frame_set_sent(FrameSetPacket &frameSet) -> void {
    if (!frameSet.remove_unreliable_frames())
        return;

    RetransmissionEntry entry{
        .packet = std::move(frameSet),
        .sentAt = std::chrono::steady_clock::now(),
    };

    sentFrameSetPackets.emplace(frameSet.sequence_number(), std::move(entry));
}

auto Connection::on_ack(const ACKPacket &ack) -> void {
    on_packet_received();

    for (const Record &record : ack.records) {
        if (record.isSingle && record.sequenceNumber) {
            sentFrameSetPackets.erase(*record.sequenceNumber);
            continue;
        }

        if (!record.startSequenceNumber && !record.endSequenceNumber)
            continue;

        for (uint24_t i = *record.startSequenceNumber; i < *record.endSequenceNumber; i++)
            sentFrameSetPackets.erase(i);
    }
}

auto Connection::on_nack(const NACKPacket &nack) -> std::vector<FrameSetPacket> {
    on_packet_received();
    
    std::vector<FrameSetPacket> frames;
    for (const Record &record : nack.records) {
        if (record.isSingle && record.sequenceNumber) {
            frames.push_back(sentFrameSetPackets.at(*record.sequenceNumber).packet);
            continue;
        }

        if (!record.startSequenceNumber && !record.endSequenceNumber)
            continue;

        for (uint24_t i = *record.startSequenceNumber; i < *record.endSequenceNumber; i++)
            frames.push_back(sentFrameSetPackets.at(i).packet);
    }
    
    return frames;
}

auto Connection::on_packet_received() -> void {
    lastReceivedTime = std::chrono::steady_clock::now();
}


} // namespace Firework::Networking::RakNet
