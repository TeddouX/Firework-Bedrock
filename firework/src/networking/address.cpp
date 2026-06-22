#include "address.hpp"

#include <format>

namespace Firework::Networking
{
    
AddressInfo::AddressInfo(std::string ipAddr, std::uint16_t port, AddressFamily family)
    : _ipAddr{ipAddr}
    , _port{port}
    , _family{family}
{}

auto AddressInfo::ipAddr() const -> const std::string & { 
    return _ipAddr; 
}

auto AddressInfo::port() const -> const std::uint16_t & { 
    return _port; 
}

auto AddressInfo::family() const -> const AddressFamily & { 
    return _family; 
}

auto AddressInfo::to_string() const -> std::string {
    if (_family == AddressFamily::IPv4)
        return std::format("{}:{}", _ipAddr, _port);
    else
        return std::format("[{}]:{}", _ipAddr, _port);
}

auto AddressInfo::operator==(const AddressInfo &other) const noexcept -> bool {
    return _port == other._port 
        && _ipAddr == other._ipAddr 
        && _family == other._family;
}


} // namespace Firework::Networking
