#pragma once
#include <vector>

#include "../firework.hpp"


namespace Firework
{

namespace Networking::RakNet { enum class PacketType : std::uint8_t; }

template <IsIntegral _Integral>
constexpr const std::uint8_t *get_int_bytes(const _Integral &value) {
    if constexpr (std::is_same_v<_Integral, uint24_t>)
        return value.get_bytes();
    return reinterpret_cast<const std::uint8_t *>(std::addressof(value));
}

class BinaryWriter {
public:
    BinaryWriter(std::size_t size);

    template <IsIntegral T>
    auto write_integral(const T &val) -> void;
    auto write_u8(std::uint8_t val) -> void;
    auto write_packet_type(Networking::RakNet::PacketType packetType) -> void;
    auto write_string(const std::string &string) -> void;
    auto write_bytes(const std::vector<std::uint8_t> &bytes) -> void;

    template <size_t _Size>
    auto write_bytes(const std::uint8_t (&bytes)[_Size]) -> void;

    auto get_data() const -> const std::vector<std::uint8_t> & { return _data; }

private:
    std::vector<std::uint8_t> _data;
};



template <IsIntegral T>
auto BinaryWriter::write_integral(const T &val) -> void {
    const uint8_t *intBytes = get_int_bytes(val);
    _data.insert(_data.end(), intBytes, intBytes + sizeof(T));
}

template <size_t _Size>
auto BinaryWriter::write_bytes(const std::uint8_t (&bytes)[_Size]) -> void {
    _data.insert(_data.end(), bytes, bytes + sizeof(bytes));
}
    
} // namespace Firework
