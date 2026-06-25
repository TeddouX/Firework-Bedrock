#pragma once
#include <map>
#include <functional>
#include <any>
#include <vector>
#include <cstdint>
#include <memory>
#include <optional>

#include "bedrock_packet.hpp"


namespace Firework::Networking::BP
{
    
// Call this after the packet was defined
#define REGISTER_PACKET(packet) \
    inline static PacketRegistrar<packet> __registrar_##packet{}

class PacketRegistry {
public:
    PacketRegistry() = default;

    template <IsPacket _Packet>
    static auto register_packet(const PacketType &type) -> void {
        PacketEntry packetEntry{};

        packetEntry.encodeFunc = [&](std::unique_ptr<IBedrockPacket> packet, BinaryWriter &writer) -> bool {
            _Packet *ptr = dynamic_cast<_Packet *>(packet.get());
            if (!ptr)
                return false;

            _Packet::PacketCodec::encode(*ptr, writer);
            return true;
        };

        packetEntry.decodeFunc = [&](BinaryReader &reader) -> std::unique_ptr<IBedrockPacket> {
            std::optional<_Packet> packetOpt = _Packet::PacketCodec::decode(reader);
            if (!packetOpt.has_value())
                return nullptr;

            return std::make_unique<_Packet>(std::move(packetOpt.value()));
        };

        _registry.emplace(type, std::move(packetEntry));
    }

    static auto encode_packet(std::unique_ptr<IBedrockPacket> packet, BinaryWriter &writer) -> bool {
        if (!packet)
            return false;

        auto it = _registry.find(packet->get_type());
        if (it == _registry.end())
            return false;

        return it->second.encodeFunc(std::move(packet), writer);
    }

    static auto decode_packet(const PacketType &type, BinaryReader &reader) -> std::unique_ptr<IBedrockPacket> {
        auto it = _registry.find(type);
        if (it == _registry.end())
            return nullptr;

        return it->second.decodeFunc(reader);
    }

private:
    struct PacketEntry {
        std::function<bool (std::unique_ptr<IBedrockPacket>, BinaryWriter &)> encodeFunc;
        std::function<std::unique_ptr<IBedrockPacket> (BinaryReader &)> decodeFunc;
    };

    inline static std::map<PacketType, PacketEntry> _registry{};
};

template<IsPacket _Packet>
struct PacketRegistrar {
    PacketRegistrar() {
        PacketRegistry::register_packet<_Packet>(_Packet::PACKET_TYPE);
    }
};

} // namespace Firework::Networking::BP
