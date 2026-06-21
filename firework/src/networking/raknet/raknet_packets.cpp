#include "raknet_packets.hpp"

#include "../utils/byte.hpp"
#include "../utils/ip.hpp"
#include "../utils/binary_reader.hpp"

#include <print>

namespace Firework::Networking
{
    
auto encode_string(const std::string &str, std::vector<std::uint8_t> &bytes) -> void;
auto insert_at_vec_end(const std::vector<std::uint8_t> &src, std::vector<std::uint8_t> &dest) -> void;
auto insert_at_vec_end(const std::uint8_t *src, std::size_t size, std::vector<std::uint8_t> &dest) -> void;
auto add_id(std::vector<std::uint8_t> &dest, RakNetPacketType packetType) -> void;
auto ip_to_bytes(const AddressInfo &addrInfo) -> std::vector<std::uint8_t>;

#define CREATE_DATA_VECTOR(size) std::vector<std::uint8_t> data{}; data.reserve(size)



auto UnconnectedPongPacket::encode() -> std::vector<std::uint8_t> {
    const size_t packetSizeBytes = 1    // ID
        + sizeof(echoedTime)            // Time
        + sizeof(serverGUID)            // Server GUID
        + sizeof(RAKNET_MAGIC)          // Magic
        + sizeof(std::uint8_t)          // Server ID string size
        + serverIdString.size();        // Server ID string

    CREATE_DATA_VECTOR(packetSizeBytes);

    add_id(data, RakNetPacketType::UNCONNECTED_PONG);                       // ID
    insert_at_vec_end(get_int_bytes(echoedTime), sizeof(echoedTime), data); // Time (echoed)
    insert_at_vec_end(get_int_bytes(serverGUID), sizeof(serverGUID), data); // Server GUID
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);            // Magic
    encode_string(serverIdString, data);

    return data;
}

auto ConnectionReply1Packet::encode() -> std::vector<std::uint8_t> {
    const size_t packetSizeBytes = 1
        + sizeof(RAKNET_MAGIC)
        + sizeof(serverGUID)
        + sizeof(useSecurity)
        + sizeof(securityCookie)
        + sizeof(MTU);

    CREATE_DATA_VECTOR(packetSizeBytes);

    add_id(data, RakNetPacketType::OPEN_CONNECTION_REP_1);                          // ID
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);                    // Magic
    insert_at_vec_end(get_int_bytes(serverGUID), sizeof(serverGUID), data);         // Server GUID
    data.push_back(static_cast<std::uint8_t>(useSecurity));                         // Security is handled by bedrock's protocol
    insert_at_vec_end(get_int_bytes(securityCookie), sizeof(securityCookie), data); // Cookie (unused)
    insert_at_vec_end(get_int_bytes(MTU), sizeof(MTU), data);                       // MTU

    return data;
}

auto ConnectionReply2Packet::encode() -> std::vector<std::uint8_t> {
    const size_t addressSize = clientAddress.family() == AddressFamily::IPv4 ? 7 : 29;
    const size_t packetSizeBytes = 1
        + sizeof(RAKNET_MAGIC)  // Magic
        + sizeof(serverGUID)    // Server GUID
        + addressSize           // 7 for IPv4; 29 for IPv6
        + sizeof(MTU)           // MTU
        + sizeof(useSecurity);  // Use security

    CREATE_DATA_VECTOR(packetSizeBytes);

    add_id(data, RakNetPacketType::OPEN_CONNECTION_REP_2);
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);            // Magic
    insert_at_vec_end(get_int_bytes(serverGUID), sizeof(serverGUID), data); // Server GUID
    insert_at_vec_end(ip_to_bytes(clientAddress), data);                    // Client IP
    insert_at_vec_end(get_int_bytes(MTU), sizeof(MTU), data);               // MTU
    data.push_back(static_cast<std::uint8_t>(useSecurity));

    return data;
}

auto FrameSetPacket::is_reliable(Reliability reliability) -> bool {
    switch (reliability) {
        using enum Reliability;
        case Reliable:
        case ReliableOrdered:
        case ReliableSequenced:
        case ReliableWithAckReceipt:
        case ReliableOrderedWithAckReceipt:
            return true;
        
        default: return false;
    }
}

auto FrameSetPacket::is_sequenced(Reliability reliability) -> bool {
    switch (reliability) {
        using enum Reliability;
        case UnreliableSequenced:
        case ReliableSequenced:
            return true;
        
        default: return false;
    }
}

auto FrameSetPacket::is_ordered(Reliability reliability) -> bool {
    switch (reliability) {
        using enum Reliability;
        case UnreliableSequenced:
        case ReliableOrdered:
        case ReliableSequenced:
        case ReliableOrderedWithAckReceipt:
            return true;
        
        default: return false;
    }
}

auto FrameSetPacket::from_packet(const UDPPacket &packet) -> std::optional<FrameSetPacket> {
    BinaryReader reader{packet.data(), packet.dataSize()};
    const std::uint8_t* data = packet.data();

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
        frame.reliability = static_cast<Reliability>(reliabilityBits);
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
            frame.compoundSize = reader.read_integral<std::int32_t>();
            if (!frame.compoundSize) return std::nullopt;
            
            frame.compoundID = reader.read_integral<std::int16_t>();
            if (!frame.compoundID) return std::nullopt;
            
            frame.fragmentIdx = reader.read_integral<std::int32_t>();
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

FrameSetPacket::FrameSetPacket(Reliability reliability, std::vector<const uint8_t *> packets, size_t packetsTotalSize) {
    // TODO: implement FrameSetPacket constructor
}

// Returns each packet that needs to be sent in case of splitting if data is too big
auto FrameSetPacket::encode() -> std::vector<std::vector<std::uint8_t>> {
    return {};
}



auto encode_string(const std::string &str, std::vector<std::uint8_t> &bytes) -> void {
    auto strBytes = reinterpret_cast<const uint8_t *>(str.c_str());
    auto strSize = static_cast<std::uint16_t>(str.size());
    strSize = host_to_network(strSize);

    auto strSizeBytes = reinterpret_cast<const std::uint8_t *>(&strSize);
    bytes.insert(
        bytes.end(),
        strSizeBytes,
        strSizeBytes + sizeof(strSize)
    );

    bytes.insert(
        bytes.end(), 
        strBytes, 
        strBytes + str.size()
    );
}

auto insert_at_vec_end(const std::vector<std::uint8_t> &src, std::vector<std::uint8_t> &dest) -> void {
    dest.insert(dest.end(), src.begin(), src.end());
}

auto insert_at_vec_end(const std::uint8_t *src, std::size_t size, std::vector<std::uint8_t> &dest) -> void {
    dest.insert(dest.end(), src, src + size);
}

auto add_id(std::vector<std::uint8_t> &dest, RakNetPacketType packetType) -> void {
    dest.push_back(static_cast<std::uint8_t>(packetType));
}

auto ip_to_bytes(const AddressInfo &addrInfo) -> std::vector<std::uint8_t> {
    std::uint16_t port = addrInfo.port();
    const std::string &ipStr = addrInfo.ipAddr();
    AddressFamily family = addrInfo.family();
    bool IPv6 = family == AddressFamily::IPv6; 

    std::vector<std::uint8_t> ipBytes;
    ipBytes.reserve(IPv6 ? 16 : 4);
    if (IPv6) {
        std::array<std::uint8_t, 16> bytes = IPv6_string_to_bytes(ipStr); 
        ipBytes.insert(ipBytes.begin(), bytes.begin(), bytes.end());
    }
    else {
        std::array<std::uint8_t, 4> bytes = IPv4_string_to_bytes(ipStr); 
        ipBytes.insert(ipBytes.begin(), bytes.begin(), bytes.end());
    }

    port = network_to_host(port);

    std::vector<std::uint8_t> bytes;
    bytes.reserve(IPv6 ? 29 : 7);
    
    bytes.push_back(static_cast<std::uint8_t>(family));

    if (IPv6) {
        // Address family
        insert_at_vec_end(get_int_bytes<std::uint16_t>(23), sizeof(std::uint16_t), bytes);
        // Port
        insert_at_vec_end(get_int_bytes(port), sizeof(port), bytes);
        // Flow 
        insert_at_vec_end(get_int_bytes<std::uint32_t>(0), sizeof(std::uint32_t), bytes); 
    } 

    // IP bytes
    insert_at_vec_end(ipBytes, bytes);

    if (IPv6)
        // Scope ID 
        insert_at_vec_end(get_int_bytes<std::uint32_t>(0), sizeof(std::uint32_t), bytes); 
    else
        // Port
        insert_at_vec_end(get_int_bytes(port), sizeof(port), bytes);

    return bytes;
}

} // namespace Firework::Networking
