#include "address.hpp"

#include <format>

namespace Firework::Networking
{
    
Address::Address(std::string ipAddr, std::uint16_t port, AddressFamily family)
    : _ipAddr{ipAddr}
    , _port{port}
    , _family{family}
{}

auto Address::ipAddr() const -> const std::string & { 
    return _ipAddr; 
}

auto Address::port() const -> const std::uint16_t & { 
    return _port; 
}

auto Address::family() const -> const AddressFamily & { 
    return _family; 
}

auto Address::to_string() const -> std::string {
    if (_family == AddressFamily::IPv4)
        return std::format("{}:{}", _ipAddr, _port);
    else
        return std::format("[{}]:{}", _ipAddr, _port);
}

auto Address::operator==(const Address &other) const noexcept -> bool {
    return _port == other._port 
        && _ipAddr == other._ipAddr 
        && _family == other._family;
}


} // namespace Firework::Networking
