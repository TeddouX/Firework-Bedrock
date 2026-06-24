#pragma once
#include <vector>

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
concept HasCodec = requires(const _Type& obj, const std::vector<std::uint8_t>& data, std::size_t off) {
    { Codec<_Type>::encode(obj) } -> std::same_as<std::vector<std::uint8_t>>;
    { Codec<_Type>::decode(data, off) } -> std::same_as<_Type>;
};

template <typename _Type>
concept CodecOrSkip = HasCodec<_Type> || is_skip<_Type>::value;


// For all integral types
template <std::integral _Type>
class Codec<_Type> {
public:
    static auto encode(const _Type &value) -> std::vector<std::uint8_t> {
        std::vector<std::uint8_t> bytes(sizeof(_Type));
        std::memcpy(bytes.data(), &value, sizeof(_Type));
        return bytes;
    }

    static auto decode(const std::vector<std::uint8_t> &data, std::size_t off) -> _Type {
        _Type value;
        std::memcpy(&value, data.data() + off, sizeof(_Type));
        return value;
    }
};

} // namespace Firework
