#pragma once
#include <cstdint>
#include <optional>
#include <span>

#include "byte.hpp"

namespace Firework
{
    
class BinaryReader {
public:
    BinaryReader(const std::uint8_t *data, size_t size);

    auto remaining() const -> std::size_t;
    auto offset() const -> std::size_t;

    template <IsIntegral T>
    auto read_integral() -> std::optional<T>;
    auto read_u8() -> std::optional<std::uint8_t>;
    auto read_bytes(std::size_t nBytes) -> std::optional<std::span<const std::uint8_t>>;

    auto advance(std::size_t nBytes) -> bool;

private:
    const std::uint8_t *_data;
    std::size_t _size;
    std::size_t _offset;
};



template <IsIntegral T>
auto BinaryReader::read_integral() -> std::optional<T> {
    if (_offset + sizeof(T) > _size)
        return std::nullopt;

    T val = int_from_bytes<T>(_data + _offset);
    _offset += sizeof(T);

    return val;
}

} // namespace Firework
