#pragma once
#include "bedrock_packet.hpp"
#include "packet_registry.hpp"
#include "../../../core/codec/object_codec.hpp"

namespace Firework::Networking::BP
{

class CS_RequestNetworkSettings: public IBedrockPacket {
public:
    static constexpr auto PACKET_TYPE = PacketType::REQUEST_NETWORK_SETTINGS;
    
    using PacketCodec = ObjectCodec<
        CS_RequestNetworkSettings, 
        BigEndian<std::int32_t>
    >;

    std::int32_t protocolVersion;

    auto handle(BedrockClient &client, BedrockServer &server) -> void override {

    }

    auto get_type() const -> PacketType override {
        return PACKET_TYPE;
    }
};

REGISTER_PACKET(CS_RequestNetworkSettings);

} // namespace Firework::Networking::BP