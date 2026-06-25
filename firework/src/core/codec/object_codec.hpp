#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include <cstring>

#include "codec.hpp"
#include "default_codecs.hpp"


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
    BinaryWriter &writer
) -> std::size_t;

template <CodecOrSkip _Value>
auto decode_value(
    BinaryReader &reader,
    BinaryWriter &writer
) -> bool;



template <typename _Obj, CodecOrSkip... _Values>
class ObjectCodec {
public:
    inline static constexpr std::size_t VALUES_SIZE = values_size<_Values...>();

    static_assert(sizeof(_Obj) != VALUES_SIZE,
        "All values in an ObjectCodec should correspond to fields in the object"
    );

    static_assert(std::is_default_constructible_v<_Obj>,
        "_Obj must be default constructible"
    );

    static auto encode(const _Obj &obj, BinaryWriter &writer) -> std::vector<std::uint8_t>;
    static auto decode(BinaryReader &reader) -> std::optional<_Obj>;

private:
    inline static constexpr std::size_t BASE_OFFSET = std::is_polymorphic_v<_Obj> ? sizeof(void*) : 0;
};



template <typename _Obj, CodecOrSkip... _Values>
auto ObjectCodec<_Obj, _Values...>::encode(const _Obj &obj, BinaryWriter &writer) -> std::vector<std::uint8_t> {
    // Skip over the vtable (requires more testing)
    auto objDataPtr = reinterpret_cast<const std::uint8_t *>(&obj) + BASE_OFFSET;

    std::size_t off = 0;
    ((off += encode_value<_Values>(objDataPtr, off, writer)),...);

    return writer.get_data();
}

template <typename _Obj, CodecOrSkip... _Values>
auto ObjectCodec<_Obj, _Values...>::decode(BinaryReader &reader) -> std::optional<_Obj> {
    _Obj obj{};

    BinaryWriter writer{VALUES_SIZE};

    if (!(decode_value<_Values>(reader, writer) && ...))
        return std::nullopt;

    const std::uint8_t* objData = writer.get_data().data();
    void* objWritePtr = reinterpret_cast<std::uint8_t *>(&obj) + BASE_OFFSET;

    std::memcpy(objWritePtr, objData, VALUES_SIZE);

    return obj;
}

template <CodecOrSkip _Value>
auto encode_value(
    const std::uint8_t *objData,
    std::size_t objDataOff,
    BinaryWriter &writer
) -> std::size_t {
    if constexpr (is_skip<_Value>::value) {
        return value_size<_Value>();
    }
    else {
        _Value val;
        std::memcpy(&val, objData + objDataOff, sizeof(_Value));

        Codec<_Value>::encode(writer, val);

        return sizeof(_Value);
    }
}

template <CodecOrSkip _Value>
auto decode_value(
    BinaryReader &reader,
    BinaryWriter &writer
) -> bool {
    constexpr std::size_t valueSize = value_size<_Value>();

    if constexpr (is_skip<_Value>::value) {
        using skipped_type = typename skip_type<_Value>::type;
        skipped_type def{};

        auto defDataPtr = reinterpret_cast<const std::uint8_t *>(&def);
        writer.write_bytes({defDataPtr, defDataPtr + valueSize});
    } else {
        std::optional<_Value> decodedOpt = Codec<_Value>::decode(reader);
        if (!decodedOpt.has_value())
            return false;

        const _Value &decoded = decodedOpt.value();
        auto decodedDataPtr = reinterpret_cast<const std::uint8_t *>(&decoded);
        writer.write_bytes({decodedDataPtr, decodedDataPtr + valueSize});
    }

    return true;
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
