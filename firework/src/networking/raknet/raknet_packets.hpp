#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../address.hpp"

namespace Firework::RakNet
{
    
constexpr std::uint8_t RAKNET_MAGIC[] = { 0x00, 0xff, 0xff, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 
                                          0xfd, 0xfd, 0xfd, 0xfd, 0x12, 0x34, 0x56, 0x78 };

enum class RakNetPacketType : std::uint8_t {
    UNCONNECTED_PING_1          = 0x01,
    UNCONNECTED_PING_2          = 0x02,
    UNCONNECTED_PONG            = 0x1c,
    CONNECTED_PING              = 0x00,
    CONNECTED_PONG              = 0x03,
    OPEN_CONNECTION_REQ_1       = 0x05,
    OPEN_CONNECTION_REP_1       = 0x06,
    OPEN_CONNECTION_REQ_2       = 0x07,
    OPEN_CONNECTION_REP_2       = 0x08,
    CONNECTION_REQUEST          = 0x09,
    CONNECTION_REQUEST_ACCEPTED = 0x10,
    NEW_INCOMING_CONNECTION     = 0x13,
    DISCONNECT                  = 0x15,
    INCOMPATIBLE_PROTOCOL       = 0x19,

    FRAME_SET                   = 0x80,
    ACK                         = 0xC0,
    NACK                        = 0xA0,
};

struct UnconnectedPongPacket {
    std::uint64_t echoedTime;
    std::uint64_t serverGUID;
    std::string serverIdString;

    auto encode() -> std::vector<std::uint8_t>;
};

struct ConnectionReply1Packet {
    std::uint64_t serverGUID;
    bool useSecurity;
    std::int32_t securityCookie;
    std::uint16_t MTU;

    auto encode() -> std::vector<std::uint8_t>;
};

struct ConnectionReply2Packet {
    std::uint64_t serverGUID;
    AddressInfo clientAddress;
    std::uint16_t MTU;
    bool useSecurity;
    
    auto encode() -> std::vector<std::uint8_t>;
};

} // namespace Firework::RakNet
