#include "raknet_packets.hpp"

#include "../utils/byte.hpp"
#include "../utils/ip.hpp"

#include <print>

namespace Firework::RakNet
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


auto FrameSetPacket::from_packet(const UDPPacket &packet) -> FrameSetPacket {
    
}

FrameSetPacket::FrameSetPacket(Reliability reliability, std::vector<const uint8_t *> packets, size_t packetsTotalSize) {

}

// Returns each packet that needs to be sent in case of splitting if data is too big
auto FrameSetPacket::encode() -> std::vector<std::vector<std::uint8_t>> {

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

} // namespace Firework::RakNet
