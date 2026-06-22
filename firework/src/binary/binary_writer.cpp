#include "binary_writer.hpp"

namespace Firework
{
    
BinaryWriter::BinaryWriter(std::size_t size) 
    : _data{} {
    _data.reserve(size);
}

auto BinaryWriter::write_u8(std::uint8_t val) -> void {
    _data.push_back(val);
}

auto BinaryWriter::write_packet_type(Networking::RakNetPacketType packetType) -> void {
    _data.push_back(static_cast<std::uint8_t>(packetType));
}

auto BinaryWriter::write_string(const std::string &string) -> void {
    auto strBytes = reinterpret_cast<const uint8_t *>(string.c_str());
    _data.insert(
        _data.end(), 
        strBytes, 
        strBytes + string.size()
    );
}

auto BinaryWriter::write_bytes(const std::vector<std::uint8_t> &bytes) -> void {
    _data.insert(_data.end(), bytes.begin(), bytes.end());
}

} // namespace Firework
