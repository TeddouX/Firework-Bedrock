#pragma once
#include "../core/logger.hpp"

namespace Firework::Networking
{

constexpr std::size_t MAX_PACKET_SIZE = 1492ULL;
    
inline static Firework::Logger LOGGER{"Firework", "Networking"};

} // namespace Firework::Networking

