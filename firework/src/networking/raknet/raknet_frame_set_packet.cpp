#include "raknet_frame_set_packet.hpp"

#include "../../binary/binary_reader.hpp"
#include "../../binary/binary_writer.hpp"
#include "../utils/byte.hpp"
#include "raknet_connection.hpp"

namespace Firework::Networking::RakNet
{
    
auto FrameSetPacket::is_reliable(Frame::Reliability reliability) -> bool {
    switch (reliability) {
        using enum Frame::Reliability;
        case Reliable:
        case ReliableOrdered:
        case ReliableSequenced:
        case ReliableWithAckReceipt:
        case ReliableOrderedWithAckReceipt:
            return true;
        
        default: return false;
    }
}

auto FrameSetPacket::is_sequenced(Frame::Reliability reliability) -> bool {
    switch (reliability) {
        using enum Frame::Reliability;
        case UnreliableSequenced:
        case ReliableSequenced:
            return true;
        
        default: return false;
    }
}

auto FrameSetPacket::is_ordered(Frame::Reliability reliability) -> bool {
    switch (reliability) {
        using enum Frame::Reliability;
        case UnreliableSequenced:
        case ReliableOrdered:
        case ReliableSequenced:
        case ReliableOrderedWithAckReceipt:
            return true;
        
        default: return false;
    }
}

auto FrameSetPacket::from_packet(const std::vector<std::uint8_t> &data) -> std::optional<FrameSetPacket> {
    BinaryReader reader{data};

    // Skip ID
    if (!reader.advance(1))
        return std::nullopt;

    auto sequenceNumberOpt = reader.read_integral<uint24_t>(); 
    if (!sequenceNumberOpt) return std::nullopt;
    uint24_t sequenceNumber = *sequenceNumberOpt;

    FrameSetPacket result{};
    result._sequenceNumber = sequenceNumber;

    // Read all frames
    while (reader.remaining() > 0) {
        Frame frame{};

        auto flagsOpt = reader.read_u8();
        if (!sequenceNumberOpt) return std::nullopt;
        std::uint8_t flags = *flagsOpt;
        
        std::uint8_t reliabilityBits = (flags & 0b11100000) >> 5;
        frame.reliability = static_cast<Frame::Reliability>(reliabilityBits);
        frame.isFragmented = flags & 0b00010000;

        frame.isReliable  = is_reliable(frame.reliability);
        frame.isSequenced = is_sequenced(frame.reliability);
        frame.isOrdered   = is_ordered(frame.reliability);

        auto lengthBitsOpt = reader.read_integral<std::uint16_t>();
        if (!lengthBitsOpt) return std::nullopt;
        frame.payloadSizeBits = network_to_host(*lengthBitsOpt);

        if (frame.isReliable) {
            frame.reliableFrameIndex = reader.read_integral<uint24_t>();
            if (!frame.reliableFrameIndex) return std::nullopt;
        }

        if (frame.isSequenced) {
            frame.sequencedFrameIndex = reader.read_integral<uint24_t>();
            if (!frame.sequencedFrameIndex) return std::nullopt;
        }

        if (frame.isOrdered) {
            frame.orderedFrameIndex = reader.read_integral<uint24_t>();
            if (!frame.orderedFrameIndex) return std::nullopt;

            frame.orderChannel = reader.read_u8();
            if (!frame.orderChannel) return std::nullopt;
        }

        if (frame.isFragmented) {
            frame.compoundSize = reader.read_integral<std::uint32_t>();
            if (!frame.compoundSize) return std::nullopt;
            
            frame.compoundID = reader.read_integral<std::uint16_t>();
            if (!frame.compoundID) return std::nullopt;
            
            frame.fragmentIdx = reader.read_integral<std::uint32_t>();
            if (!frame.fragmentIdx) return std::nullopt;

            frame.compoundSize = network_to_host(*frame.compoundSize);
            frame.compoundID   = network_to_host(*frame.compoundID);
            frame.fragmentIdx  = network_to_host(*frame.fragmentIdx);
        }
        
        // Equivalent to ceil(frame.payloadSizeBits / 8)
        std::size_t payloadSize = (static_cast<std::size_t>(frame.payloadSizeBits) + 7) / 8;
        auto payload = reader.read_bytes(payloadSize);
        if (!payload) return std::nullopt;
        
        frame.payload.assign(payload->begin(), payload->end());
        
        result._frames.push_back(std::move(frame));
    }

    return result;
}

auto FrameSetPacket::from_partial_frames(std::vector<PartialFrame> &packets, Connection &connection) -> std::vector<FrameSetPacket> {
    std::vector<FrameSetPacket> frameSetPackets{};

    std::size_t maxFrameSize = connection.MTU 
        - 28                // IP (20) + UDP (8)
        - 1                 // Frame set packet ID
        - sizeof(uint24_t); // Frame set packet sequence number

    std::size_t mergedFramesSize = 0;
    FrameSetPacket mergedFrameSetPacket{};
    for (PartialFrame &partialFrame : packets) {
        std::size_t headerSize = Frame::COMMON_HEADER_SIZE;
        std::size_t payloadSize = partialFrame.payload.size();

        Frame frame{};
        frame.reliability = partialFrame.reliability;
        frame.isReliable = is_reliable(frame.reliability);
        frame.isSequenced = is_sequenced(frame.reliability);
        frame.isOrdered = is_ordered(frame.reliability);

        if (frame.isReliable)
            headerSize += Frame::RELIABLE_HEADER_SIZE;
        
        if (frame.isSequenced) {
            headerSize += Frame::SEQUENCED_HEADER_SIZE;
            frame.sequencedFrameIndex = connection.nextSequencedFrameIdx++;
        }

        if (frame.isSequenced || frame.isOrdered) {
            if (!frame.isSequenced)
                headerSize += Frame::ORDERED_HEADER_SIZE;
            
            frame.orderChannel = partialFrame.orderChannel;
            frame.orderedFrameIndex = connection.orderingChannels[partialFrame.orderChannel]++;
        }

        std::size_t maxPayloadSize = maxFrameSize - headerSize;

        // The payload needs to be seperated into multiple frame set packets
        if (payloadSize > maxPayloadSize) {
            // Each packet now gets the fragmented header
            maxPayloadSize -= Frame::FRAGMENT_HEADER_SIZE;

            std::size_t numFullPackets = payloadSize / maxPayloadSize;
            std::size_t rem = payloadSize % maxPayloadSize;
            
            std::vector<std::vector<std::uint8_t>> payloads;
            for (std::size_t i = 0; i < numFullPackets; ++i) {
                payloads.emplace_back(
                    partialFrame.payload.begin() + i * maxPayloadSize, 
                    partialFrame.payload.begin() + (i + 1) * maxPayloadSize
                );
            }

            if (rem > 0) {
                payloads.emplace_back(
                    partialFrame.payload.begin() + numFullPackets * maxPayloadSize, 
                    partialFrame.payload.begin() + numFullPackets * maxPayloadSize + rem
                );
            }

            std::uint16_t compoundID = connection.nextCompoundID++;
            for (std::uint32_t i = 0; i < payloads.size(); i++) {
                std::vector<std::uint8_t> &payload = payloads[i];

                // Copy header
                Frame fragmentedFrame = frame;
                fragmentedFrame.isFragmented = true;
                fragmentedFrame.payloadSizeBits = payload.size() * 8;
                
                // Each fragment doesn't share the same reliableFrameIndex (to be verified) 
                if (fragmentedFrame.isReliable)
                    fragmentedFrame.reliableFrameIndex = connection.nextReliableFrameIdx++;

                fragmentedFrame.compoundSize = static_cast<std::uint32_t>(payloads.size());
                fragmentedFrame.compoundID = compoundID;
                fragmentedFrame.fragmentIdx = i;

                fragmentedFrame.payload = std::move(payload);

                FrameSetPacket packet{};
                packet._frames.push_back(std::move(fragmentedFrame));
                packet._sequenceNumber = connection.nextSequenceNumber++;

                frameSetPackets.push_back(std::move(packet));
            }

            continue;
        }

        if (frame.isReliable)
            frame.reliableFrameIndex = connection.nextReliableFrameIdx++;

        frame.payloadSizeBits = payloadSize * 8;
        frame.payload = std::move(partialFrame.payload);
        
        std::size_t frameSize = headerSize + payloadSize;
        if (mergedFramesSize + frameSize < maxFrameSize) {
            // Add this frame to the merged packet
            mergedFrameSetPacket._frames.push_back(std::move(frame));
            mergedFrameSetPacket._sequenceNumber = connection.nextSequenceNumber++;
        }
        else {
            // Add the old, too big merged frame packet to the list
            if (!mergedFrameSetPacket._frames.empty())
                frameSetPackets.push_back(std::move(mergedFrameSetPacket));
            
            // Start a new merged packet
            mergedFrameSetPacket = FrameSetPacket{};
            mergedFrameSetPacket._frames.push_back(std::move(frame));
            mergedFrameSetPacket._sequenceNumber = connection.nextSequenceNumber++;
        }

        mergedFramesSize += frameSize;
    }

    if (mergedFrameSetPacket._frames.size() > 0)
        frameSetPackets.push_back(std::move(mergedFrameSetPacket));

    return frameSetPackets;
}

auto FrameSetPacket::encode(Connection &connection) const -> std::vector<std::uint8_t> {
    // As to not cause too many reallocations
    BinaryWriter writer{connection.MTU};
    
    writer.write_packet_type(PacketType::FRAME_SET);
    // See: https://minecraft.wiki/w/RakNet#Frame_Set_Packet
    writer.write_integral(_sequenceNumber); 

    for (const Frame &frame : _frames) {
        std::uint8_t flags = 0;
        flags |= static_cast<std::uint8_t>(frame.reliability) << 5;
        flags |= static_cast<std::uint8_t>(frame.isFragmented) << 4;
    
        writer.write_u8(flags);
        writer.write_integral(host_to_network(frame.payloadSizeBits));

        if (frame.isReliable && frame.reliableFrameIndex)
            writer.write_integral(*frame.reliableFrameIndex);
    
        if (frame.isSequenced && frame.sequencedFrameIndex)
            writer.write_integral(*frame.sequencedFrameIndex);

        if (frame.isOrdered && frame.orderedFrameIndex && frame.orderChannel) {
            writer.write_integral(*frame.orderedFrameIndex);
            writer.write_integral(*frame.orderChannel);
        }

        if (frame.isFragmented && frame.compoundSize && frame.compoundID && frame.fragmentIdx) {
            writer.write_integral(*frame.compoundSize);
            writer.write_integral(*frame.compoundID);
            writer.write_integral(*frame.fragmentIdx);
        }

        writer.write_bytes(frame.payload);
    }

    std::size_t udpPacketSize = writer.get_data().size() + 28;
    if (udpPacketSize > connection.MTU)
        LOGGER.warn("Packet size exceeded MTU, might cause splitting ({} > {}), addr: {}", 
            udpPacketSize,
            connection.MTU,
            connection.address.to_string()
        );

    return writer.get_data();
}

// Removes every unreliable frames, returns false if empty after operation
auto FrameSetPacket::remove_unreliable_frames() -> bool {
    for (std::size_t i = 0; i < _frames.size(); i++) {
        const Frame &frame = _frames[i];
        // Swap and pop, better time complexity :)
        if (!frame.isReliable) {
            _frames[i] = std::move(_frames.back());
            _frames.pop_back();
        }
    }

    return !_frames.empty();
}

} // namespace Firework::Networking::RakNet
