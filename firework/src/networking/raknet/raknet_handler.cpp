#include "raknet_handler.hpp"

#include <sstream>
#include <print>
#include <random>

#include "../../firework.hpp"
#include "../utils/byte.hpp"
#include "raknet_packets.hpp"

namespace Firework::RakNet
{
    
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
            return;
        
        case OPEN_CONNECTION_REQ_1:
            handle_connection_req_1(packet);
            return;
        
        case OPEN_CONNECTION_REQ_2:
            handle_connection_req_2(packet);
            return;
    }

    if (id & static_cast<uint8_t>(RakNetPacketType::FRAME_SET) && id < 0x90) {
        std::vector<uint8_t> frameSet = decode_frame_set(packet);

        // Do smthing with it

        return;
    }

    std::println("Unhandled packet: '{}'", id);
}

auto RakNetHandler::handle_unconnected_ping(const UDPPacket &packet) -> void {
    UnconnectedPongPacket replyPacket{};
    replyPacket.echoedTime = int_from_bytes<std::uint64_t>(packet.data + 1);
    replyPacket.serverGUID = _guid;
    replyPacket.serverIdString = _cachedServerIDString;

    _udpServer->send(replyPacket.encode(), packet.addrInfo);

    std::println("Sent UnconnectedPong");
}

auto RakNetHandler::handle_connection_req_1(const UDPPacket &packet) -> void {  
    ConnectionReply1Packet replyPacket{};
    replyPacket.serverGUID = _guid;
    replyPacket.useSecurity = false;
    replyPacket.securityCookie = 0;
    
    //                                            ID  MAGIC                  PROTOCOL VERSION
    std::uint16_t paddingSize = packet.dataSize - 1 - sizeof(RAKNET_MAGIC) - 1; // This is the size of the padded zeroes
    std::uint16_t mtu = paddingSize + 46; // (see https://minecraft.wiki/w/RakNet#Open_Connection_Request_1 for the +46 explanation)
    replyPacket.MTU = mtu;


    if (_udpServer->send(replyPacket.encode(), packet.addrInfo)) {
        // On successful reply, add a new connection
        RakNetConnection connection{};
        connection.address = packet.addrInfo;
        connection.isFullyConnected = false;

        _openConnections.emplace(packet.addrInfo.to_string(), connection);

        std::println("Sent ConnectionReply1.");
    }
}

auto RakNetHandler::handle_connection_req_2(const UDPPacket &packet) -> void {
    ConnectionReply2Packet replyPacket{};
    replyPacket.serverGUID = _guid;
    replyPacket.clientAddress = packet.addrInfo;
    replyPacket.useSecurity = false;
    
    auto mtu = static_cast<std::uint16_t>(MAX_PACKET_SIZE);
    mtu = host_to_network(mtu);
    replyPacket.MTU = mtu;

    _udpServer->send(replyPacket.encode(), packet.addrInfo);

    std::println("Sent ConnectionReply2.");
}

auto RakNetHandler::decode_frame_set(const UDPPacket &packet) -> std::vector<uint8_t> {
    return {};
}

} // namespace Firework::RakNet
