#include "raknet_packets.hpp"

#include "../utils/byte.hpp"
#include "../utils/ip.hpp"
#include "../../binary/binary_reader.hpp"
#include "../../binary/binary_writer.hpp"

#include <print>

namespace Firework::Networking::RakNet
{

auto ip_to_bytes(const Address &addrInfo) -> std::vector<std::uint8_t>;

auto records_from_packet(BinaryReader &reader) -> std::vector<Record>;
auto encode_records(BinaryWriter &writer, const std::vector<Record> &records) -> void;


auto UnconnectedPingPacket::from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<UnconnectedPingPacket> {
    if (packet.size() != UnconnectedPingPacket::SIZE)
        return std::nullopt;

    BinaryReader reader{packet};

    PacketType packetType = *reader.read_packet_type();
    if (packetType != PacketType::UNCONNECTED_PING_1 && packetType != PacketType::UNCONNECTED_PING_2)
        return std::nullopt;

    UnconnectedPingPacket finalPacket{};

    finalPacket.time = network_to_host(*reader.read_integral<std::uint64_t>());
    reader.advance(MAGIC_SIZE);
    finalPacket.clientGUID = network_to_host(*reader.read_integral<std::uint64_t>());

    return finalPacket;
}

auto UnconnectedPongPacket::encode() const -> std::vector<std::uint8_t> {
    const size_t packetSizeBytes = 1
        + sizeof(echoedTime)
        + sizeof(serverGUID)
        + MAGIC_SIZE
        + sizeof(std::uint16_t)
        + serverIdString.size();

    BinaryWriter writer{packetSizeBytes};

    writer.write_packet_type(PacketType::UNCONNECTED_PONG);
    writer.write_integral(host_to_network(echoedTime));
    writer.write_integral(host_to_network(serverGUID));
    writer.write_bytes(MAGIC);
    writer.write_integral(host_to_network(static_cast<std::uint16_t>(serverIdString.size())));
    writer.write_string(serverIdString);

    return writer.get_data();
}

auto OpenConnectionRequest1Packet::from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<OpenConnectionRequest1Packet> {
    if (packet.size() < OpenConnectionRequest1Packet::MIN_SIZE)
        return std::nullopt;

    BinaryReader reader{packet};
    
    PacketType packetType = *reader.read_packet_type();
    if (packetType != PacketType::OPEN_CONNECTION_REQ_1)
        return std::nullopt;

    OpenConnectionRequest1Packet finalPacket{};

    reader.advance(MAGIC_SIZE);
    finalPacket.protocolVersion = *reader.read_u8();

    //                                          ID  MAGIC        PROTOCOL VERSION
    std::uint16_t paddingSize = packet.size() - 1 - MAGIC_SIZE - 1; // This is the size of the padded zeroes
    finalPacket.MTU = paddingSize + 46; // (see https://minecraft.wiki/w/RakNet#Open_Connection_Request_1 for the +46 explanation)

    return finalPacket;
}

auto OpenConnectionReply1Packet::encode() const -> std::vector<std::uint8_t> {
    BinaryWriter writer{OpenConnectionReply1Packet::SIZE};

    writer.write_packet_type(PacketType::OPEN_CONNECTION_REP_1);
    writer.write_bytes(MAGIC);
    writer.write_integral(host_to_network(serverGUID));
    writer.write_u8(0u); // Use Security: We use bedrock's protocol security
    writer.write_integral(host_to_network(MTU));

    return writer.get_data();
}

auto OpenConnectionRequest2Packet::from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<OpenConnectionRequest2Packet> {
    if (packet.size() != OpenConnectionRequest2Packet::SIZE)
        return std::nullopt;

    BinaryReader reader{packet};
    
    PacketType packetType = *reader.read_packet_type();
    if (packetType != PacketType::OPEN_CONNECTION_REQ_2)
        return std::nullopt;

    OpenConnectionRequest2Packet finalPacket{};

    reader.advance(MAGIC_SIZE);

    std::span<const std::uint8_t> serverAddress = *reader.read_bytes(sizeof(finalPacket.serverAddress));
    std::copy(serverAddress.begin(), serverAddress.begin(), finalPacket.serverAddress.end());

    finalPacket.MTU = network_to_host(*reader.read_integral<std::uint16_t>());
    finalPacket.clientGUID = network_to_host(*reader.read_integral<std::uint64_t>());

    return finalPacket;
}

auto ConnectionRequestPacket::from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<ConnectionRequestPacket> {
    if (packet.size() != ConnectionRequestPacket::SIZE)
        return std::nullopt;
    
    BinaryReader reader{packet};

    PacketType packetType = *reader.read_packet_type();
    if (packetType != PacketType::CONNECTION_REQUEST)
        return std::nullopt;

    ConnectionRequestPacket finalPacket{};

    finalPacket.clientGUID = network_to_host(*reader.read_integral<std::uint64_t>());
    finalPacket.sendTime = network_to_host(*reader.read_integral<std::uint64_t>());

    return finalPacket;
}

auto ConnectionRequestAcceptedPacket::encode() const -> std::vector<std::uint8_t> {
    BinaryWriter writer{ConnectionRequestAcceptedPacket::SIZE};

    writer.write_packet_type(PacketType::CONNECTION_REQUEST_ACCEPTED);
    writer.write_bytes(ip_to_bytes(clientAddress));
    writer.write_integral((std::uint16_t)0);

    Address addrInfo{"255.255.255.255", 19132, AddressFamily::IPv4};
    std::vector<std::uint8_t> addrInfoBytes = ip_to_bytes(addrInfo);
    for (int i = 0; i < 10; i++)
        writer.write_bytes(addrInfoBytes);

    writer.write_integral(connectionRequestTime);
    writer.write_integral(sendTime);

    return writer.get_data();
}
 
auto OpenConnectionReply2Packet::encode() const -> std::vector<std::uint8_t> {
    BinaryWriter writer{SIZE};

    writer.write_packet_type(PacketType::OPEN_CONNECTION_REP_2);
    writer.write_bytes(MAGIC);
    writer.write_integral(host_to_network(serverGUID));
    writer.write_bytes(ip_to_bytes(clientAddress));
    writer.write_integral(host_to_network(MTU));
    writer.write_u8(0u); // Use Security: We use bedrock's protocol security

    return writer.get_data();
}

Record::Record(std::uint32_t rangeStart, std::uint32_t rangeEnd) {
    if (rangeStart == rangeEnd) {
        isSingle = true;
        sequenceNumber = rangeStart;
    }
    else {
        isSingle = false;
        startSequenceNumber = rangeStart;
        endSequenceNumber = rangeEnd;
    }
}

auto ACKPacket::from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<ACKPacket> {
    if (packet.empty())
        return std::nullopt;

    BinaryReader reader{packet};

    PacketType packetType = *reader.read_packet_type();
    if (packetType != PacketType::ACK)
        return std::nullopt;

    ACKPacket finalPacket{};
    finalPacket.records = records_from_packet(reader);

    if (finalPacket.records.empty())
        return std::nullopt;

    return std::move(finalPacket);
}

auto ACKPacket::encode() const -> std::vector<std::uint8_t> {
    const size_t packetSizeBytes = 1
        + sizeof(std::uint16_t)
        + records.size();

    BinaryWriter writer{packetSizeBytes};
    writer.write_packet_type(PacketType::ACK);

    encode_records(writer, records);

    return writer.get_data();
}

auto NACKPacket::from_packet(const std::vector<std::uint8_t> &packet) -> std::optional<NACKPacket> {
    if (packet.empty())
        return std::nullopt;

    BinaryReader reader{packet};

    PacketType packetType = *reader.read_packet_type();
    if (packetType != PacketType::NACK)
        return std::nullopt;

    NACKPacket finalPacket{};
    finalPacket.records = records_from_packet(reader);

    if (finalPacket.records.empty())
        return std::nullopt;

    return std::move(finalPacket);
}

auto NACKPacket::encode() const -> std::vector<std::uint8_t> {
    const size_t packetSizeBytes = 1
        + sizeof(std::uint16_t)
        + records.size();

    BinaryWriter writer{packetSizeBytes};
    writer.write_packet_type(PacketType::NACK);

    encode_records(writer, records);

    return writer.get_data();
}


auto ip_to_bytes(const Address &addrInfo) -> std::vector<std::uint8_t> {
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

    BinaryWriter writer{IPv6 ? 29uz : 7uz};
    
    writer.write_u8(static_cast<std::uint8_t>(family));

    if (IPv6) {
        writer.write_integral(23);      // IPv6 Address family
        writer.write_integral(port);    // IPv6 Port
        writer.write_integral(0);       // IPv6 Flow 
    } 

    writer.write_bytes(ipBytes); // IP bytes

    if (IPv6) writer.write_integral(0);     // IPv6 Scope ID 
    else      writer.write_integral(port);  // IPv4 Port

    return writer.get_data();
}

auto records_from_packet(BinaryReader &reader) -> std::vector<Record> {
    auto recordsCountOpt = reader.read_integral<std::uint16_t>();
    if (!recordsCountOpt) return {};
    std::uint16_t recordCount = network_to_host(*recordsCountOpt);

    std::vector<Record> records{};
    records.reserve(recordCount);

    for (std::uint16_t i = 0; i < recordCount; i++) {
        Record rec{};

        auto isSingleOpt = reader.read_u8();
        if (!isSingleOpt) return {};
        if (*isSingleOpt > 1) return {};
        
        rec.isSingle = *isSingleOpt;

        if (rec.isSingle) {
            rec.sequenceNumber = reader.read_integral<uint24_t>();
            if (!rec.sequenceNumber) return {};
        }
        else {
            rec.startSequenceNumber = reader.read_integral<uint24_t>();
            if (!rec.startSequenceNumber) return {};
            rec.endSequenceNumber = reader.read_integral<uint24_t>();
            if (!rec.endSequenceNumber) return {};
        }
        
        records.push_back(rec);
    }

    return records;
}

auto encode_records(BinaryWriter &writer, const std::vector<Record> &records) -> void {
    std::uint16_t recordsSize = static_cast<std::uint16_t>(records.size());
    writer.write_integral(host_to_network(recordsSize));

    for (const Record &record : records) {
        writer.write_u8(record.isSingle);

        if (record.isSingle && record.sequenceNumber)
            writer.write_integral(*record.sequenceNumber);
        else if (!record.isSingle && record.startSequenceNumber && record.endSequenceNumber) {
            writer.write_integral(*record.startSequenceNumber);
            writer.write_integral(*record.endSequenceNumber);
        }
    }
}


} // namespace Firework::Networking::RakNet
