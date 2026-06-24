#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "../address.hpp"
#include "../networking.hpp"

namespace Firework::Networking::Socket
{
    
class UDPPacket {
public:
    constexpr UDPPacket()
        : _addrInfo{}
        , _data{}
    {}

    UDPPacket(Address addrInfo, const std::vector<std::uint8_t> &data)
        : _addrInfo(addrInfo)
        , _data{data}
    {}

    constexpr auto addrInfo() const -> const Address & { return _addrInfo; }
    constexpr auto data() const -> const std::vector<std::uint8_t> & { return _data; }
    constexpr auto dataSize() const -> std::size_t { return _data.size(); }

private:
    Address                     _addrInfo;
    std::vector<std::uint8_t>   _data;
};

} // namespace Firework::Networking::Socket
