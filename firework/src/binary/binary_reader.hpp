#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "../firework.hpp"

namespace Firework
{

namespace Networking { enum class RakNetPacketType : std::uint8_t; }

template <IsIntegral _Integral>
constexpr _Integral int_from_bytes(const std::uint8_t *value) {
    if constexpr (std::is_same_v<_Integral, uint24_t>)
        return uint24_t{value};
    return *reinterpret_cast<const _Integral *>(value);
}

class BinaryReader {
public:
    BinaryReader(const std::vector<std::uint8_t> &data);

    auto remaining() const -> std::size_t;
    auto offset() const -> std::size_t;

    template <IsIntegral T>
    auto read_integral() -> std::optional<T>;
    auto read_u8() -> std::optional<std::uint8_t>;
    auto read_packet_type() -> std::optional<Networking::RakNetPacketType>;
    auto read_bytes(std::size_t nBytes) -> std::optional<std::span<const std::uint8_t>>;

    auto advance(std::size_t nBytes) -> bool;

private:
    const std::vector<std::uint8_t> &_data;
    std::size_t _offset;
};



template <IsIntegral T>
auto BinaryReader::read_integral() -> std::optional<T> {
    if (_offset + sizeof(T) > _data.size())
        return std::nullopt;

    T val = int_from_bytes<T>(_data.data() + _offset);
    _offset += sizeof(T);

    return val;
}

} // namespace Firework
