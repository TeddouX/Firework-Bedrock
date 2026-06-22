#include "binary_reader.hpp"

namespace Firework
{
    
BinaryReader::BinaryReader(const std::vector<std::uint8_t> &data)
    : _data{data}
    , _offset{0}
{}

auto BinaryReader::remaining() const -> std::size_t { 
    return _data.size() - _offset; 
}

auto BinaryReader::offset() const -> std::size_t { 
    return _offset; 
}

auto BinaryReader::read_u8() -> std::optional<std::uint8_t> {
    if (_offset + 1 > _data.size()) 
        return std::nullopt;
    return _data[_offset++];
}

auto BinaryReader::read_packet_type() -> std::optional<Networking::RakNetPacketType> {
    auto u8 = read_u8();
    if (!u8)
        return std::nullopt;
    return static_cast<Networking::RakNetPacketType>(*u8);
}


auto BinaryReader::read_bytes(std::size_t nBytes) -> std::optional<std::span<const std::uint8_t>> {
    if (_offset + nBytes > _data.size()) 
        return std::nullopt;

    std::span<const std::uint8_t> result(_data.data() + _offset, nBytes);
    _offset += nBytes;

    return result;
}

auto BinaryReader::advance(std::size_t nBytes) -> bool {
    if (_offset + nBytes > _data.size()) 
        return false;

    _offset += nBytes;
    return true;
}

} // namespace Firework
