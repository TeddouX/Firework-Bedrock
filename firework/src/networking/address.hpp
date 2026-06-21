#pragma once
#include <cstdint>
#include <string>
#include <format>

namespace Firework::Networking
{
    
enum class AddressFamily : std::uint8_t {
    IPv4 = 0x4,
    IPv6 = 0x6, 
};

class AddressInfo {
public:
    constexpr AddressInfo() = default;
    constexpr AddressInfo(std::string ipAddr, std::uint16_t port, AddressFamily family)
        : _ipAddr{ipAddr}
        , _port{port}
        , _family{family}
    {
    }

    constexpr auto ipAddr() const -> const std::string & { return _ipAddr; }
    constexpr auto port() const -> const std::uint16_t & { return _port; }
    constexpr auto family() const -> const AddressFamily & { return _family; }

    constexpr std::string to_string() const {
        if (_family == AddressFamily::IPv4)
            return std::format("{}:{}", _ipAddr, _port);
        else
            return std::format("[{}]:{}", _ipAddr, _port);
    }

private:
    std::string     _ipAddr;
    std::uint16_t   _port;
    AddressFamily   _family;
};

} // namespace Firework::Networking
