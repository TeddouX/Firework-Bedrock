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

    constexpr uint24_t() noexcept 
        : _data{0}
    {}

    uint24_t(const std::uint8_t *bytes) {
        std::memcpy(_data, bytes, SIZE_BYTES);
    }

    constexpr uint24_t(const uint24_t &val) noexcept {
        *this = val;
    }

    constexpr uint24_t(const std::uint32_t &val) noexcept {
        *this = val;
    }

    constexpr auto get_bytes() const noexcept -> const std::uint8_t * { 
        return _data; 
    }

    constexpr auto byteswap() const -> uint24_t {
        const std::uint8_t swapped[SIZE_BYTES] = { _data[2], _data[1], _data[0] };
        return uint24_t{swapped};
    }

    constexpr auto is_newer_or_equals(const uint24_t &other) const noexcept -> bool {
        // Take into account wraparounds
        std::uint32_t diff = static_cast<std::uint32_t>(*this - other);
        return diff <= (MAX / 2);
    }

    constexpr auto is_newer(const uint24_t &other) const noexcept -> bool {
        return *this != other && is_newer_or_equals(other);
    }

    // --------------------------------
    // |     CONVERSION OPERATORS     |
    // --------------------------------

    constexpr operator std::uint32_t() const noexcept {
        return static_cast<std::uint32_t>(_data[2]) << 16
             | static_cast<std::uint32_t>(_data[1]) << 8
             | static_cast<std::uint32_t>(_data[0]);
    }

    constexpr operator bool() const noexcept {
        return static_cast<std::uint32_t>(*this) != 0;
    }

    constexpr auto operator-() const noexcept -> uint24_t {
        return uint24_t(-static_cast<std::uint32_t>(*this));
    }

    // ----------------------------
    // |     ASSIGN OPERATORS     |
    // ----------------------------

    constexpr auto operator=(const uint24_t &other) noexcept -> uint24_t & {
        for (size_t i = 0; i < SIZE_BYTES; i++) 
            _data[i] = other._data[i];
        return *this;
    }
    
    constexpr auto operator=(const std::uint32_t &other) noexcept -> uint24_t & {
        _data[0] = static_cast<std::uint8_t>(other & 0xFF);
        _data[1] = static_cast<std::uint8_t>((other >> 8) & 0xFF);
        _data[2] = static_cast<std::uint8_t>((other >> 16) & 0xFF);
        return *this;
    }

    // -------------------------------------------------------
    // |     ARITHMETIC OPERATIONS BETWEEN two UINT24_Ts     |
    // -------------------------------------------------------

    constexpr auto operator+(const uint24_t &other) const noexcept -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            + static_cast<std::uint32_t>(other)
        );
    }
    
    constexpr auto operator-(const uint24_t &other) const noexcept -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            - static_cast<std::uint32_t>(other)
        );
    }

    constexpr auto operator*(const uint24_t &other) const noexcept -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            * static_cast<std::uint32_t>(other)
        );
    }

    constexpr auto operator/(const uint24_t &other) const noexcept -> uint24_t {
        return uint24_t(
            static_cast<std::uint32_t>(*this) 
            / static_cast<std::uint32_t>(other)
        );
    }

    // --------------------------------------------------------------------
    // |     ARITHMETIC OPERATIONS BETWEEN A UINT24_T AND AN UINT32_T     |
    // --------------------------------------------------------------------

    constexpr auto operator+(const std::uint32_t &other) const noexcept -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) + other);
    }
    
    constexpr auto operator-(const std::uint32_t &other) const noexcept -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) - other);
    }

    constexpr auto operator*(const std::uint32_t &other) const noexcept -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) * other);
    }

    constexpr auto operator/(const std::uint32_t &other) const noexcept -> uint24_t {
        return uint24_t(static_cast<std::uint32_t>(*this) / other);
    }

    // --------------------------------------------------------------
    // |     ARITHMETIC ASSIGN OPERATIONS BETWEEN two UINT24_Ts     |
    // --------------------------------------------------------------

    constexpr auto operator+=(const uint24_t &other) noexcept -> uint24_t & {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const uint24_t &other) noexcept -> uint24_t & {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const uint24_t &other) noexcept -> uint24_t & {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const uint24_t &other) noexcept -> uint24_t & {
        *this = *this / other;
        return *this;
    }

    // ---------------------------------------------------------------------------
    // |     ARITHMETIC ASSIGN OPERATIONS BETWEEN A UINT24_T AND AN UINT32_T     |
    // ---------------------------------------------------------------------------

    constexpr auto operator+=(const std::uint32_t &other) noexcept -> uint24_t & {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const std::uint32_t &other) noexcept -> uint24_t & {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const std::uint32_t &other) noexcept -> uint24_t & {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const std::uint32_t &other) noexcept -> uint24_t & {
        *this = *this / other;
        return *this;
    }

    // ----------------------------------------------
    // |     INCREMENT AND DECREMENT OPERATIONS     |
    // ----------------------------------------------

    // Prefix
    constexpr auto operator++() noexcept -> uint24_t & {
        *this += 1u;
        return *this;
    }

    // Postfix
    constexpr auto operator++(int) noexcept -> uint24_t {
        uint24_t temp = *this;
        *this += 1u;
        return temp;
    }

    // Prefix
    constexpr auto operator--() noexcept -> uint24_t & {
        *this -= 1u;
        return *this;
    }

    // Postfix
    constexpr auto operator--(int) noexcept -> uint24_t {
        uint24_t temp = *this;
        *this -= 1u;
        return temp;
    }

    // ------------------------------------------------------
    // |     COMPARISON OPERATORS BETWEEN TWO UINT24_Ts     |
    // ------------------------------------------------------
    constexpr auto operator==(const uint24_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) == static_cast<std::uint32_t>(other);
    }

    constexpr auto operator!=(const uint24_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) != static_cast<std::uint32_t>(other);
    }

    constexpr auto operator>=(const uint24_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) >= static_cast<std::uint32_t>(other);
    }

    constexpr auto operator<=(const uint24_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) <= static_cast<std::uint32_t>(other);
    }

    constexpr auto operator>(const uint24_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) > static_cast<std::uint32_t>(other);
    }

    constexpr auto operator<(const uint24_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) < static_cast<std::uint32_t>(other);
    }

    // ------------------------------------------------------------------
    // |     COMPARISON OPERATORS BETWEEN A UINT24_T AND A UINT32_T     |
    // ------------------------------------------------------------------
    constexpr auto operator==(const std::uint32_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) == other;
    }

    constexpr auto operator!=(const std::uint32_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) != other;
    }

    constexpr auto operator>=(const std::uint32_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) >= other;
    }

    constexpr auto operator<=(const std::uint32_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) <= other;
    }

    constexpr auto operator>(const std::uint32_t &other) const noexcept -> bool {
        return static_cast<std::uint32_t>(*this) > other;
    }

    constexpr auto operator<(const std::uint32_t &other) const noexcept -> bool {
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
