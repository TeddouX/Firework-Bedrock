#pragma once
#include "../core/logger.hpp"

namespace Firework::Networking
{

#define RAKNET_PROTOCOL_VERSION 11

// Maximum packet size racknet will send
constexpr std::size_t MAX_PACKET_SIZE = 1492zu;

inline static Firework::Logger LOGGER{"Firework", "Networking"};

} // namespace Firework::Networking

