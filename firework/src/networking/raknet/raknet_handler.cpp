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
        + sizeof(std::uint64_t) 
        + sizeof(RAKNET_MAGIC) 
        + _cachedServerIDString.size();

    std::vector<std::uint8_t> unconnectedPongData{};
    unconnectedPongData.reserve(packetSizeBytes);

    // ID
    unconnectedPongData.push_back(static_cast<std::uint8_t>(RakNetPacketType::UNCONNECTED_PONG));
    
    // Time
    const uint8_t *timeUnconnectedPing = packet.data + 1;
    insert_at_vec_end(timeUnconnectedPing, sizeof(std::uint64_t), unconnectedPongData);

    // Server GUID
    auto guidBytes = reinterpret_cast<const uint8_t *>(&_guid);
    insert_at_vec_end(guidBytes, sizeof(_guid), unconnectedPongData);

    // Magic
    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), unconnectedPongData);

    // Server ID String
    encode_string(_cachedServerIDString, unconnectedPongData);

    _udpServer->send(unconnectedPongData, packet.addrInfo);

    std::println("Sent UnconnectedPong");
}

auto RakNetHandler::handle_connection_req_1(const UDPPacket &packet) -> void {
    const size_t packetSizeBytes = 1    // ID 
        + sizeof(RAKNET_MAGIC)          // MAGIC
        + sizeof(std::uint64_t)         // Server GUID
        + sizeof(bool)                  // Use security
        + sizeof(std::int32_t)          // Cookie
        + sizeof(std::uint16_t);        // Answer with the MTU 
    
    std::vector<std::uint8_t> data{};
    data.reserve(packetSizeBytes);

    data.push_back(static_cast<std::uint8_t>(RakNetPacketType::OPEN_CONNECTION_REP_1));

    insert_at_vec_end(RAKNET_MAGIC, sizeof(RAKNET_MAGIC), data);

    auto guidBytes = reinterpret_cast<const uint8_t *>(&_guid);
    insert_at_vec_end(guidBytes, sizeof(_guid), data);

    data.push_back(static_cast<std::uint8_t>(false)); // Security is handled by bedrock's protocol
    
    const std::int32_t zero = 0;
    auto zeroIntBytes = reinterpret_cast<const uint8_t *>(&zero);
    insert_at_vec_end(zeroIntBytes, sizeof(zero), data);

    //                                    ID  MAGIC                  PROTOCOL VERSION
    std::uint16_t mtu = packet.dataSize - 1 - sizeof(RAKNET_MAGIC) - 1; // This is the size of the padded zeroes
    mtu += 46; // (see https://minecraft.wiki/w/RakNet#Open_Connection_Request_1 for the +46 explanation)

    auto mtuBytes = reinterpret_cast<const std::uint8_t *>(&mtu);
    insert_at_vec_end(mtuBytes, sizeof(mtu), data);

    _udpServer->send(data, packet.addrInfo);

    std::println("Sent ConnectionReply1.");
}

auto RakNetHandler::handle_connection_req_2(const UDPPacket &packet) -> void {
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

} // namespace Firework
