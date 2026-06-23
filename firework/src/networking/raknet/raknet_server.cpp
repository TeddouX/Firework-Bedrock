#include "raknet_server.hpp"

#include <sstream>
#include <print>
#include <random>

#include "../../firework.hpp"
#include "../utils/byte.hpp"
#include "raknet_packets.hpp"

namespace Firework::Networking::RakNet
{
    
auto build_ranges(const std::set<uint24_t> &nums) -> std::vector<Record>;



Server::Server(const ServerProperties &serverProperties, std::shared_ptr<UDPServer> udpServer) 
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

    _startTime = std::chrono::steady_clock::now();
}

auto Server::properties_to_string() -> const std::string & {
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

auto Server::update_server_properties(const ServerProperties &serverProperties) -> void {
    _serverProperties = serverProperties;
    _serverIDStringDirty = true;
}

auto Server::handle_packet(const UDPPacket &packet) -> void {
    if (packet.dataSize() < 1)
        return;
    
    std::uint8_t id = packet.data()[0];
    if (id & static_cast<uint8_t>(PacketType::FRAME_SET) && id < 0x90) {
        LOGGER.debug("Received frame set packet");

        std::vector<Frame> frames = decode_frame_set(packet);
        for (Frame frame : frames)
            handle_packet(packet.addrInfo(), frame.payload);
    
        return;
    }

    handle_packet(packet.addrInfo(), packet.data());
}

auto Server::update_connections() -> void {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    for (auto &[_, conn]: _openConnections) {
        if (!conn.missingSequenceNumbers.empty()) {
            NACKPacket packet{};
            packet.records = build_ranges(conn.missingSequenceNumbers);

            _udpServer->send(packet.encode(), conn.address);
            
            conn.missingSequenceNumbers.clear();

            LOGGER.debug("Sent NACK");
        }

        if (!conn.pendingACKsequenceNumbers.empty()) {
            std::chrono::nanoseconds ackInterval = now - conn.lastOutgoingACKTime;

            if (ackInterval > Connection::ACK_FLUSH_INTERVAL || 
                conn.pendingACKsequenceNumbers.size() >= Connection::MAX_PENDING_ACKS_PACKETS
            ) {
                ACKPacket packet{};
                packet.records = build_ranges(conn.pendingACKsequenceNumbers);

                conn.lastOutgoingACKTime = now;
                conn.pendingACKsequenceNumbers.clear();

                _udpServer->send(packet.encode(), conn.address);
                LOGGER.debug("Sent ACK");
            }
        }

        for (auto &[_, retransmissionEntry] : conn.sentFrameSetPackets) {
            std::chrono::nanoseconds retransmissionInterval = now - retransmissionEntry.sentAt;
            if (retransmissionInterval > Connection::RETRANSMISSION_TIMEOUT) {
                send_frame_set(conn, retransmissionEntry.packet);
                retransmissionEntry.sentAt = now;
            }
        }
    }
}

auto Server::handle_unconnected_ping(const AddressInfo &addrInfo, const UnconnectedPingPacket &packet) -> void {
    UnconnectedPongPacket replyPacket{};
    replyPacket.echoedTime = packet.time;
    replyPacket.serverGUID = _guid;
    replyPacket.serverIdString = _cachedServerIDString;

    _udpServer->send(replyPacket.encode(), addrInfo);

    LOGGER.debug("Sent UnconnectedPong");
}

auto Server::handle_open_connection_req_1(const AddressInfo &addrInfo, const OpenConnectionRequest1Packet &packet) -> void {  
    OpenConnectionReply1Packet replyPacket{};
    replyPacket.serverGUID = _guid;
    
    std::uint16_t connectionMTU = std::min(packet.MTU, static_cast<std::uint16_t>(MAX_PACKET_SIZE));
    replyPacket.MTU = connectionMTU;

    if (_udpServer->send(replyPacket.encode(), addrInfo)) {
        // On successful reply, add a new connection
        Connection connection{};
        connection.address = addrInfo;
        connection.isFullyConnected = false;
        connection.MTU = connectionMTU;

        _openConnections.emplace(addrInfo, connection);

        LOGGER.debug("Sent ConnectionReply1.");
    }
}

auto Server::handle_open_connection_req_2(const AddressInfo &addrInfo, const OpenConnectionRequest2Packet &packet) -> void {
    Connection *conn = get_connection(addrInfo);
    if (!conn) return;

    OpenConnectionReply2Packet replyPacket{};
    replyPacket.serverGUID = _guid;
    replyPacket.clientAddress = addrInfo;
    replyPacket.MTU = conn->MTU;

    _udpServer->send(replyPacket.encode(), addrInfo);

    LOGGER.debug("Sent ConnectionReply2.");
}

auto Server::handle_connection_request(const AddressInfo &addrInfo, const ConnectionRequestPacket &packet) -> void {
    Connection *conn = get_connection(addrInfo);
    if (!conn) return;

    ConnectionRequestAcceptedPacket replyPacket{};
    replyPacket.clientAddress = addrInfo;
    replyPacket.connectionRequestTime = packet.sendTime;

    std::chrono::steady_clock::duration timeDiff = std::chrono::steady_clock::now() - _startTime;
    replyPacket.sendTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count();

    PartialFrame partialFrame{};
    partialFrame.reliability = Frame::Reliability::ReliableOrdered;
    partialFrame.payload = replyPacket.encode();
    partialFrame.orderChannel = FIREWORK_RAKNET_RESERVED_CHANNEL;

    send_in_frame_set(*conn, partialFrame);

    LOGGER.debug("Sent ConnectionRequestAccepted.");
}

auto Server::handle_disconnect(const AddressInfo &addrInfo) -> void {
    _openConnections.erase(addrInfo);

    LOGGER.debug("{} disconnected.", addrInfo.to_string());
}

auto Server::handle_ack(const AddressInfo &addrInfo, const ACKPacket &packet) -> void {
    Connection *conn = get_connection(addrInfo);
    if (!conn) return;

    conn->on_ack(packet);

    LOGGER.debug("Received ACK");
}

auto Server::handle_nack(const AddressInfo &addrInfo, const NACKPacket &packet) -> void {
    Connection *conn = get_connection(addrInfo);
    if (!conn) return;

    std::vector<FrameSetPacket> frameSets = conn->on_nack(packet);
    send_frame_sets(*conn, frameSets);

    LOGGER.debug("Received NACK");
}

auto Server::decode_frame_set(const UDPPacket &packet) -> std::vector<Frame> {
    auto frameSetPacket = FrameSetPacket::from_packet(packet.data());
    if (!frameSetPacket) {
        LOGGER.warn("Failed to decode frame set packet.");
        return {};
    }

    auto it = _openConnections.find(packet.addrInfo());
    if (it == _openConnections.end())
        return {};
    
    Connection &connection = it->second;
    connection.lastReceivedTime = std::chrono::steady_clock::now();
    
    connection.update_sequence(frameSetPacket->sequence_number());

    std::vector<Frame> packets;
    for (Frame frame : frameSetPacket->frames()) {
        std::vector<Frame> frames = connection.update_frame_level_data(frame); 
        if (frames.empty())
            continue;

        packets.insert(packets.end(), frames.begin(), frames.end());
    }

    return packets;
}

auto Server::send_in_frame_set(Connection &connection, PartialFrame &partialFrame) -> bool {
    std::vector<PartialFrame> partialFrames{{std::move(partialFrame)}};
    std::vector<FrameSetPacket> frameSets = FrameSetPacket::from_partial_frames(partialFrames, connection);
    
    return send_frame_sets(connection, frameSets);
}

auto Server::send_frame_sets(Connection &connection, std::vector<FrameSetPacket> &frameSets) -> bool {
    std::vector<std::vector<uint8_t>> packets{};
    for (const FrameSetPacket &frameSet : frameSets)
        packets.push_back(frameSet.encode(connection));

    if (_udpServer->send_all(packets, connection.address)) {
        for (FrameSetPacket &frameSet : frameSets)
            connection.on_frame_set_sent(frameSet);

        return true;
    }

    return false;
}

auto Server::send_frame_set(Connection &connection, const FrameSetPacket &frameSet) -> bool {
    std::vector<uint8_t> packet = frameSet.encode(connection);
    return _udpServer->send(packet, connection.address);
}

auto Server::handle_packet(const AddressInfo &addrInfo, const std::vector<std::uint8_t> &data) -> void {
    if (data.size() < 1)
        return;

    uint8_t id = data[0];
    auto packetType = static_cast<PacketType>(id);

    switch (packetType)
    {
        using enum PacketType;
        case UNCONNECTED_PING_1:
        case UNCONNECTED_PING_2: {
            auto packet = UnconnectedPingPacket::from_packet(data);
            if (!packet) return;

            handle_unconnected_ping(addrInfo, *packet);
            return;
        }
        
        case OPEN_CONNECTION_REQ_1: {
            auto packet = OpenConnectionRequest1Packet::from_packet(data);
            if (!packet) return;

            handle_open_connection_req_1(addrInfo, *packet);
            return;
        }

        case OPEN_CONNECTION_REQ_2: {
            auto packet = OpenConnectionRequest2Packet::from_packet(data);
            if (!packet) return;

            handle_open_connection_req_2(addrInfo, *packet);
            return;
        }

        case CONNECTION_REQUEST: {
            auto packet = ConnectionRequestPacket::from_packet(data);
            if (!packet) return;

            handle_connection_request(addrInfo, *packet);
            return;
        }

        case CONNECTED_PING: {
            // TODO
            
            return;
        }

        case NEW_INCOMING_CONNECTION: {
            // TODO

            return;
        }

        case ACK: {
            auto packet = ACKPacket::from_packet(data);
            if (!packet) return;

            handle_ack(addrInfo, *packet);
            return;
        }

        case NACK: {
            auto packet = NACKPacket::from_packet(data);
            if (!packet) return;
            
            handle_nack(addrInfo, *packet);
            return;
        }

        case DISCONNECT: {
            handle_disconnect(addrInfo);

            return;
        }
    }

    LOGGER.warn("Unhandled packet {:02X}", id);
}

auto Server::get_connection(const AddressInfo &addrInfo) -> Connection * {
    auto it = _openConnections.find(addrInfo);
    if (it == _openConnections.end())
        return nullptr;

    return &it->second;
}



auto build_ranges(const std::set<uint24_t> &nums) -> std::vector<Record> {
    std::vector<Record> records{};
    records.reserve(nums.size());

    std::uint32_t rangeStart = UINT32_MAX;
    std::uint32_t rangeEnd = UINT32_MAX;
    for (uint24_t num : nums) {
        if (rangeStart == UINT32_MAX) {
            rangeStart = num;
            rangeEnd = num;
            continue;
        }

        if ((rangeEnd + 1) == static_cast<std::uint32_t>(num)) {
            rangeEnd++;
            continue;
        }
        
        records.emplace_back(rangeStart, rangeEnd);
        
        rangeStart = UINT32_MAX;
        rangeEnd = UINT32_MAX;
    }

    if (rangeStart != UINT32_MAX) {
        if (rangeEnd != UINT32_MAX) {
            records.emplace_back(rangeStart, rangeEnd);
            return records;
        }

        records.emplace_back(rangeStart, rangeStart);
    }

    return records;
}

} // namespace Firework::Networking::RakNet
