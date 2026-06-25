#pragma once
#include <vector>
#include <map>

#include "../address.hpp"
#include "packet/bedrock_packet.hpp"
#include "packet/packet_registry.hpp"


namespace Firework::Networking::BP
{

class BedrockServer {

public:
    BedrockServer() = default;

    auto handle_packet(const Address &addr, const std::vector<std::uint8_t> &packet) -> void;

private:
};

} // namespace Firework::Networking::BP
