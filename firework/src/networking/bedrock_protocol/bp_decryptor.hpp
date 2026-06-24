#pragma once
#include <vector>

namespace Firework::Networking::BP
{

auto zlib_decompress_packet(
    std::vector<std::uint8_t> &compressedPacket, 
    std::size_t maxSize = 1024 * 1024 // Default 1MB
) -> std::vector<std::uint8_t>;

} // namespace Firework::Networking::BP
