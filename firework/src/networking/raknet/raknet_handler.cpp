#include "raknet_handler.hpp"

#include <sstream>
#include <print>
#include <random>

#include "../../firework.hpp"
#include "../utils/byte.hpp"

namespace Firework
{
    
auto encode_string(const std::string &str, std::vector<std::uint8_t> &bytes) -> void;
auto insert_at_vec_end(const std::vector<std::uint8_t> &src, std::vector<std::uint8_t> &dest) -> void;
auto insert_at_vec_end(const std::uint8_t *src, std::size_t size, std::vector<std::uint8_t> &dest) -> void;
auto add_id(std::vector<std::uint8_t> &dest, RakNetPacketType packetType) -> void;
auto ip_to_bytes(const std::string &ip, std::uint16_t port, std::uint8_t type) -> std::vector<std::uint8_t>;

#define CREATE_DATA_VECTOR(size) std::vector<std::uint8_t> data{}; data.reserve(size)

RakNetHandler::RakNetHandler(const ServerProperties &serverProperties, std::shared_ptr<UDPServer> udpServer) 
    : _guid{0}
    , _serverProperties{serverProperties}
    , _cachedServerIDString{}
    , _serverIDStringDirty{true}
    , _udpServer{udpServer} {
    // initialise _cachedServerIDString 
    std::random_device rd;
    std::mt19937_64 rng(rd());
    _guid = rng();
    
    properties_to_string();
}

auto RakNetHandler::properties_to_string() -> const std::string & {
    // Edition (MCPE or MCEE for Education Edition);MOTD line 1;Protocol Version;Version Name;Player Count;Max Player Count;Server Unique ID;MOTD line 2;Game mode;Game mode (numeric);Port (IPv4);Port (IPv6);
    if (!_serverIDStringDirty)
        return _cachedServerIDString;
    
    std::stringstream sstream;
    sstream << "MCPE"                           << ";";
    sstream << _serverProperties.motd1          << ";";
    sstream << BEDROCK_PROTOCOL_VERSION         << ";";
    sstream << BEDROCK_VERSION_NAME             << ";";
    sstream << _serverProperties.playerCount    << ";";
    sstream << _serverProperties.maxPlayerCount << ";";
    sstream << _guid                            << ";";
    sstream << _serverProperties.motd2          << ";";
    sstream << _serverProperties.gameMode       << ";";
    sstream << _serverProperties.gameModeID     << ";";
    sstream << _serverProperties.portIPv4       << ";";
    sstream << _serverProperties.portIPv6       << ";";

    _cachedServerIDString = sstream.str();

    std::println("{}", _cachedServerIDString);

    return _cachedServerIDString;
}

auto RakNetHandler::update_server_properties(const ServerProperties &serverProperties) -> void {
    _serverProperties = serverProperties;
    _serverIDStringDirty = true;
}

auto RakNetHandler::handle_packet(const UDPPacket &packet) -> void {
    if (packet.dataSize < 1)
        return;

    uint8_t id = packet.data[0];
    auto packetType = static_cast<RakNetPacketType>(id);

    std::println("Packet id 0x{:02X}", id);

    switch (packetType)
    {
        using enum RakNetPacketType;
        case UNCONNECTED_PING_1:
        case UNCONNECTED_PING_2:
            handle_unconnected_ping(packet);
            break;
        
        case CONNECTED_PING:
            break;
        
        case CONNECTED_PONG:
            break;
        
        case OPEN_CONNECTION_REQ_1:
            handle_connection_req_1(packet);
            break;
        
        case OPEN_CONNECTION_REQ_2:
            handle_connection_req_2(packet);
            break;
        
        case CONNECTION_REQUEST:
            break;
        
        case NEW_INCOMING_CONNECTION:
            break;
        
        case DISCONNECT:
            break;
    }
}

auto RakNetHandler::handle_unconnected_ping(const UDPPacket &packet) -> void {
    // ID; Time (long); Server GUID (long); Magic; Server ID String
    const size_t packetSizeBytes = 1 
        + sizeof(std::uint64_t) 
        + sizeof(_guid)
        + sizeof(RAKNET_MAGIC) 
        + _cachedServerIDString.size();

    CREATE_DATA_VECTOR(packetSizeBytes);

    add_id(data, RakNetPacketType::UNCONNECTED_PONG);                   // ID
    insert_at_vec_end(packet.data + 1, sizeof(std::uint64_t), data);    // Time (echoed)
    insert_at_vec_end(get_int_bytes(_guid), sizeof(_guid), data);       // Server GUID
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);        // Magic
    encode_string(_cachedServerIDString, data);                         // Server ID String

    _udpServer->send(data, packet.addrInfo);

    std::println("Sent UnconnectedPong");
}

auto RakNetHandler::handle_connection_req_1(const UDPPacket &packet) -> void {
    const size_t packetSizeBytes = 1    // ID 
        + sizeof(RAKNET_MAGIC)          // MAGIC
        + sizeof(_guid)                 // Server GUID
        + 1                             // Use security
        + sizeof(std::int32_t)          // Cookie
        + sizeof(std::uint16_t);        // Answer with the MTU 
    
    //                                    ID  MAGIC                  PROTOCOL VERSION
    std::uint16_t mtu = packet.dataSize - 1 - sizeof(RAKNET_MAGIC) - 1; // This is the size of the padded zeroes
    mtu += 46; // (see https://minecraft.wiki/w/RakNet#Open_Connection_Request_1 for the +46 explanation)

    CREATE_DATA_VECTOR(packetSizeBytes);

    add_id(data, RakNetPacketType::OPEN_CONNECTION_REP_1);                          // ID
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);                    // Magic
    insert_at_vec_end(get_int_bytes(_guid), sizeof(_guid), data);                   // Server GUID
    data.push_back((std::uint8_t)false);                                            // Security is handled by bedrock's protocol
    insert_at_vec_end(get_int_bytes((std::int32_t)0), sizeof(std::int32_t), data);  // Cookie (unused)
    insert_at_vec_end(get_int_bytes(mtu), sizeof(mtu), data);                       // MTU

    _udpServer->send(data, packet.addrInfo);

    std::println("Sent ConnectionReply1.");
}

auto RakNetHandler::handle_connection_req_2(const UDPPacket &packet) -> void {
    const size_t packetSizeBytes = 1
        + sizeof(RAKNET_MAGIC)  // Magic
        + sizeof(_guid)         // Server GUID
        + 7                     // 7 for IPv4; 29 for IPv6
        + sizeof(std::uint16_t) // MTU
        + 1;                    // Use security
    
    auto mtu = static_cast<std::uint16_t>(MAX_PACKET_SIZE);
    mtu = host_to_network(mtu);

    CREATE_DATA_VECTOR(packetSizeBytes);

    add_id(data, RakNetPacketType::OPEN_CONNECTION_REP_2);
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);                                // Magic
    insert_at_vec_end(get_int_bytes(_guid), sizeof(_guid), data);                               // Server GUID
    insert_at_vec_end(ip_to_bytes(packet.addrInfo.ipAddr, packet.addrInfo.port, 0x4), data);    // Client IP
    insert_at_vec_end(get_int_bytes(mtu), sizeof(mtu), data);                                   // MTU
    data.push_back((std::uint8_t)false);                                                        // Security is handled by bedrock's protocol

    _udpServer->send(data, packet.addrInfo);

    std::println("Sent ConnectionReply2.");
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

auto ip_to_bytes(const std::string &ip, std::uint16_t port, std::uint8_t type) -> std::vector<std::uint8_t> {
    std::int32_t ipNumber = 0;
    std::string currNumber = "";
    
    for (char c : ip) {
        if (c == '.') {
            ipNumber += std::stoi(currNumber);
            continue;
        }

        currNumber += c;
    }

    ipNumber = network_to_host(ipNumber);
    port = network_to_host(port);

    std::vector<std::uint8_t> bytes;
    bytes.reserve(type == 0x4 ? 7 : 29);
    
    bytes.push_back(type);
    insert_at_vec_end(get_int_bytes(ipNumber), sizeof(ipNumber), bytes);
    insert_at_vec_end(get_int_bytes(port), sizeof(port), bytes);

    return bytes;
}

} // namespace Firework
