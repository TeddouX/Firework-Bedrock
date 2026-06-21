#include "binary_reader.hpp"

namespace Firework
{
    
BinaryReader::BinaryReader(const std::uint8_t *data, std::size_t size)
    : _data{data}
    , _size{size}
    , _offset{0}
{}

auto BinaryReader::remaining() const -> std::size_t { 
    return _size - _offset; 
}

auto BinaryReader::offset() const -> std::size_t { 
    return _offset; 
}

auto BinaryReader::read_u8() -> std::optional<std::uint8_t> {
    if (_offset + 1 > _size) 
        return std::nullopt;
    return _data[_offset++];
}

auto BinaryReader::read_bytes(std::size_t nBytes) -> std::optional<std::span<const std::uint8_t>> {
    if (_offset + nBytes > _size) 
        return std::nullopt;

    std::span<const std::uint8_t> result(_data + _offset, nBytes);
    _offset += nBytes;

    return result;
}

auto BinaryReader::advance(std::size_t nBytes) -> bool {
    if (_offset + nBytes > _size) 
        return false;

    _offset += nBytes;
    return true;
}

} // namespace Firework
