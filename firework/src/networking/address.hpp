#pragma once
#include <cstdint>
#include <string>

namespace Firework::Networking
{
    
enum class AddressFamily : std::uint8_t {
    IPv4 = 0x4,
    IPv6 = 0x6, 
};

class Address {
public:
    Address() = default;
    Address(std::string ipAddr, std::uint16_t port, AddressFamily family);

    auto ipAddr() const -> const std::string &;
    auto port() const -> const std::uint16_t &;
    auto family() const -> const AddressFamily &;

    auto to_string() const -> std::string;

    auto operator==(const Address &other) const noexcept -> bool;

private:
    std::string     _ipAddr;
    std::uint16_t   _port;
    AddressFamily   _family;
};

} // namespace Firework::Networking

template<> 
struct std::hash<Firework::Networking::Address> {
    std::size_t operator()(const Firework::Networking::Address& s) const noexcept {
        return std::hash<std::string>{}(s.to_string());
    }
};
