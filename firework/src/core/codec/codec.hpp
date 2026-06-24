#pragma once
#include <vector>

#include "../../binary/binary_writer.hpp"
#include "../../binary/binary_reader.hpp"

namespace Firework
{
  
template <typename _Type>
    requires std::is_default_constructible_v<_Type>
struct Skip {};

template <typename _Type>
struct is_skip : std::false_type {};

template <typename _Type>
struct is_skip<Skip<_Type>> : std::true_type {};

template <typename _Type>
struct skip_type;

template <typename _Type>
struct skip_type<Skip<_Type>> {
    using type = _Type;
};


template <typename _Type>
class Codec;

template <typename _Type>
concept HasCodec = requires(const _Type &value, BinaryWriter &writer, BinaryReader &reader) {
    { Codec<_Type>::encode(writer, value) } -> std::same_as<void>;
    { Codec<_Type>::decode(reader) } -> std::same_as<std::optional<_Type>>;
};

template <typename _Type>
concept CodecOrSkip = HasCodec<_Type> || is_skip<_Type>::value;


// For all integral types
template <std::integral _Type>
class Codec<_Type> {
public:
    static auto encode(BinaryWriter &writer, const _Type &value) -> void {
        writer.write_integral(value);
    }

    static auto decode(BinaryReader &reader) -> std::optional<_Type> {
        return reader.read_integral<_Type>();
    }
};

} // namespace Firework
