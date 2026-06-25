#pragma once
#include <type_traits>

#include "../bedrock_client.hpp"
#include "../../../core/codec/object_codec.hpp"

namespace Firework::Networking::BP
{
 
enum class PacketType {
    REQUEST_NETWORK_SETTINGS = 0xC1,
};

class IBedrockPacket;
class BedrockServer;

template <typename _Packet>
concept IsPacket = 
    std::is_base_of_v<IBedrockPacket, _Packet> && 
    requires { 
        typename _Packet::PacketCodec; 
        _Packet::PACKET_TYPE; 
    };

class IBedrockPacket {
public:
    virtual ~IBedrockPacket() = default;

    virtual auto handle(BedrockClient &client, BedrockServer &server) -> void = 0;
    virtual auto get_type() const -> PacketType = 0;
};

} // namespace Firework::Networking::BP
