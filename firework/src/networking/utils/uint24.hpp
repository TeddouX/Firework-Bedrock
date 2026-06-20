#pragma once
#include <cstdint>
#include <string>
#include <bitset>

namespace Firework
{
    
class uint24_t {
public:
    static constexpr std::size_t SIZE_BYTES = 3;
    static constexpr std::size_t MAX = 0xFFFFFF;

    uint24_t() 
        : _data{0}
    {}

    uint24_t(const std::uint8_t *bytes, std::size_t readOff = 0ULL) {
        std::memcpy(_data, bytes + readOff, SIZE_BYTES);
    }

    uint24_t(const uint24_t &val) {
        *this = val;
    }

    uint24_t(const std::uint32_t &val) {
        *this = val;
    }

    auto get_bytes() const -> const std::uint8_t * { 
        return _data; 
    }

    auto byteswap() const -> uint24_t {
        const std::uint8_t swapped[SIZE_BYTES] = { _data[2], _data[1], _data[0] };
        return uint24_t{swapped};
    }

    // --------------------------------
    // |     CONVERSION OPERATORS     |
    // --------------------------------

    operator std::uint32_t() const {
        return static_cast<std::uint32_t>(_data[2]) << 16
             | static_cast<std::uint32_t>(_data[1]) << 8
             | static_cast<std::uint32_t>(_data[0]);
    }

    operator bool() const {
        return static_cast<std::uint32_t>(*this) != 0;
    }

    auto operator-() const -> uint24_t {
        return uint24_t(-static_cast<std::uint32_t>(*this));
    }

    // ----------------------------
    // |     ASSIGN OPERATORS     |
    // ----------------------------

    auto operator=(const uint24_t &other) -> uint24_t & {
        std::memcpy(_data, other._data, SIZE_BYTES);
        return *this;
    }
    
    auto operator=(const std::uint32_t &other) -> uint24_t & {
        auto otherBytes = reinterpret_cast<const std::uint8_t *>(other);
        std::memcpy(_data, otherBytes, SIZE_BYTES);
        return *this;
    }

    // -------------------------------------------------------
    // |     ARITHMETIC OPERATIONS BETWEEN two UINT24_Ts     |
    // -------------------------------------------------------

    auto operator+(const uint24_t &other) const -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            + static_cast<std::uint32_t>(other)
        );
    }
    
    auto operator-(const uint24_t &other) const -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            - static_cast<std::uint32_t>(other)
        );
    }

    auto operator*(const uint24_t &other) const -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            * static_cast<std::uint32_t>(other)
        );
    }

    auto operator/(const uint24_t &other) const -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            / static_cast<std::uint32_t>(other)
        );
    }

    // --------------------------------------------------------------------
    // |     ARITHMETIC OPERATIONS BETWEEN A UINT24_T AND AN UINT32_T     |
    // --------------------------------------------------------------------

    auto operator+(const std::uint32_t &other) const -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) + other);
    }
    
    auto operator-(const std::uint32_t &other) const -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) - other);
    }

    auto operator*(const std::uint32_t &other) const -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) * other);
    }

    auto operator/(const std::uint32_t &other) const -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) / other);
    }

    // --------------------------------------------------------------
    // |     ARITHMETIC ASSIGN OPERATIONS BETWEEN two UINT24_Ts     |
    // --------------------------------------------------------------

    auto operator+=(const uint24_t &other) -> uint24_t & {
        *this = *this + other;
        return *this;
    }

    auto operator-=(const uint24_t &other) -> uint24_t & {
        *this = *this - other;
        return *this;
    }

    auto operator*=(const uint24_t &other) -> uint24_t & {
        *this = *this * other;
        return *this;
    }

    auto operator/=(const uint24_t &other) -> uint24_t & {
        *this = *this / other;
        return *this;
    }

    // ---------------------------------------------------------------------------
    // |     ARITHMETIC ASSIGN OPERATIONS BETWEEN A UINT24_T AND AN UINT32_T     |
    // ---------------------------------------------------------------------------

    auto operator+=(const std::uint32_t &other) -> uint24_t & {
        *this = *this + other;
        return *this;
    }

    auto operator-=(const std::uint32_t &other) -> uint24_t & {
        *this = *this - other;
        return *this;
    }

    auto operator*=(const std::uint32_t &other) -> uint24_t & {
        *this = *this * other;
        return *this;
    }

    auto operator/=(const std::uint32_t &other) -> uint24_t & {
        *this = *this / other;
        return *this;
    }

    // ------------------------------------------------------
    // |     COMPARISON OPERATORS BETWEEN TWO UINT24_Ts     |
    // ------------------------------------------------------
    auto operator==(const uint24_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) == static_cast<std::uint32_t>(other);
    }

    auto operator!=(const uint24_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) != static_cast<std::uint32_t>(other);
    }

    auto operator>=(const uint24_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) >= static_cast<std::uint32_t>(other);
    }

    auto operator<=(const uint24_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) <= static_cast<std::uint32_t>(other);
    }

    auto operator>(const uint24_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) > static_cast<std::uint32_t>(other);
    }

    auto operator<(const uint24_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) < static_cast<std::uint32_t>(other);
    }

    // ------------------------------------------------------------------
    // |     COMPARISON OPERATORS BETWEEN A UINT24_T AND A UINT32_T     |
    // ------------------------------------------------------------------
    auto operator==(const std::uint32_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) == other;
    }

    auto operator!=(const std::uint32_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) != other;
    }

    auto operator>=(const std::uint32_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) >= other;
    }

    auto operator<=(const std::uint32_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) <= other;
    }

    auto operator>(const std::uint32_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) > other;
    }

    auto operator<(const std::uint32_t &other) const -> bool {
        return static_cast<std::uint32_t>(*this) < other;
    }

private:
    std::uint8_t _data[SIZE_BYTES];
};

} // namespace Firework

template<> 
struct std::hash<Firework::uint24_t> {
    std::size_t operator()(const Firework::uint24_t& s) const noexcept {
        return std::hash<std::uint32_t>{}(static_cast<std::uint32_t>(s));
    }
};
