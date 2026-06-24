#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include <cstring>

#include "codec.hpp"


namespace Firework
{

template <typename _Value>
constexpr auto value_size() -> std::size_t;

template <typename... _Values>
constexpr auto values_size() -> std::size_t;

template <CodecOrSkip _Value>
auto encode_value(
    const std::uint8_t *objData,
    std::size_t objDataOff,
    std::vector<std::uint8_t> &out
) -> std::size_t;

template <CodecOrSkip _Value>
auto decode_value(
    const std::vector<std::uint8_t> &data,
    std::size_t dataOff,
    std::vector<std::uint8_t> &out
) -> std::size_t;



template <typename _Obj, CodecOrSkip... _Values>
class ObjectCodec {
public:
    static constexpr std::size_t VALUES_SIZE = values_size<_Values...>();

    static_assert(sizeof(_Obj) >= VALUES_SIZE,
        "All values in an ObjectCodec should correspond to fields in the object"
    );

    static_assert(std::is_default_constructible_v<_Obj>,
        "_Obj must be default constructible"
    );

    static auto encode(const _Obj &obj) -> std::vector<std::uint8_t>;
    static auto decode(const std::vector<std::uint8_t> &data) -> _Obj;
};



template <typename _Obj, CodecOrSkip... _Values>
auto ObjectCodec<_Obj, _Values...>::encode(const _Obj &obj) -> std::vector<std::uint8_t> {
    auto objDataPtr = reinterpret_cast<const std::uint8_t *>(&obj);

    std::vector<std::uint8_t> encodedData{};
    encodedData.reserve(VALUES_SIZE);

    std::size_t off = 0;
    ((off += encode_value<_Values>(objDataPtr, off, encodedData)),...);

    return encodedData;
}

template <typename _Obj, CodecOrSkip... _Values>
auto ObjectCodec<_Obj, _Values...>::decode(const std::vector<std::uint8_t> &data) -> _Obj {
    _Obj obj{};

    std::vector<std::uint8_t> objData{};
    objData.reserve(sizeof(_Obj));

    std::size_t off = 0;
    ((off += decode_value<_Values>(data, off, objData)),...);

    std::memcpy(&obj, objData.data(), sizeof(_Obj));

    return obj;
}

template <CodecOrSkip _Value>
auto encode_value(
    const std::uint8_t *objData,
    std::size_t objDataOff,
    std::vector<std::uint8_t> &out
) -> std::size_t {
    if constexpr (is_skip<_Value>::value) {
        return value_size<_Value>();
    }
    else {
        _Value val;
        std::memcpy(&val, objData + objDataOff, sizeof(_Value));

        std::vector<std::uint8_t> valueBytes = Codec<_Value>::encode(val);
        out.insert(out.end(), valueBytes.begin(), valueBytes.end());

        return sizeof(_Value);
    }
}

template <CodecOrSkip _Value>
auto decode_value(
    const std::vector<std::uint8_t> &data,
    std::size_t dataOff,
    std::vector<std::uint8_t> &out
) -> std::size_t {
    constexpr std::size_t valueSize = value_size<_Value>();

    if constexpr (is_skip<_Value>::value) {
        using skipped_type = typename skip_type<_Value>::type;
        
        skipped_type def{};
        auto defDataPtr = reinterpret_cast<const std::uint8_t *>(&def);

        out.insert(out.end(), defDataPtr, defDataPtr + valueSize);

        return 0;
    } else {
        _Value decoded = Codec<_Value>::decode(data, dataOff);
        auto decodedDataPtr = reinterpret_cast<const std::uint8_t *>(&decoded);

        out.insert(out.end(), decodedDataPtr, decodedDataPtr + valueSize);

        return valueSize;
    }
}

template <typename _Value>
constexpr auto value_size() -> std::size_t {
    if constexpr (is_skip<_Value>::value) {
        using skipped_type = typename skip_type<_Value>::type;
        return sizeof(skipped_type);
    }
    else {
        return sizeof(_Value);
    }
}

template <typename... _Values>
constexpr auto values_size() -> std::size_t {
    std::size_t size = 0;

    ((size += value_size<_Values>()),...);

    return size;
}

} // namespace Firework
