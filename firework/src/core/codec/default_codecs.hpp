#pragma once
#include "codec.hpp"

#include "../../binary/binary_reader.hpp"
#include "../../binary/binary_writer.hpp"
#include "../../networking/utils/byte.hpp"

namespace Firework
{
    
// For all integral types
template <IsIntegral _Type>
class Codec<_Type> {
public:
    static auto encode(BinaryWriter &writer, const _Type &value) -> void {
        writer.write_integral(value);
    }

    static auto decode(BinaryReader &reader) -> std::optional<_Type> {
        return reader.read_integral<_Type>();
    }
};

template <IsIntegral _Type>
struct BigEndian {
    _Type value{};

    constexpr BigEndian() = default;
    constexpr BigEndian(_Type v) : value(v) {}
    constexpr operator _Type() const { return value; }
};

// Automatic big endian handling
template <IsIntegral _Type>
class Codec<BigEndian<_Type>> {
public:
    static auto encode(BinaryWriter &writer, const BigEndian<_Type> &value) -> void {
        writer.write_integral(Networking::host_to_network(value.value));
    }

    static auto decode(BinaryReader &reader) -> std::optional<BigEndian<_Type>> {
        std::optional<_Type> raw = reader.read_integral<_Type>();
        if (!raw.has_value())
            return std::nullopt;
        
        return BigEndian<_Type>{Networking::network_to_host(raw.value())};
    }
};

} // namespace Firework
