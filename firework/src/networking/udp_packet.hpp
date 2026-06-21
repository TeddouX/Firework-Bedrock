#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#include "address.hpp"
#include "networking.hpp"

namespace Firework::Networking
{
    
class UDPPacket {
public:
    constexpr UDPPacket()
        : _addrInfo{}
        , _data{0}
        , _dataSize{0}
    {}

    UDPPacket(AddressInfo addrInfo, const std::uint8_t *data, std::size_t dataSize)
        : _addrInfo(addrInfo)
        , _data{0}
        , _dataSize(dataSize) 
    {
        std::memcpy(_data, data, dataSize);
    }

    constexpr auto addrInfo() const -> const AddressInfo & { return _addrInfo; }
    constexpr auto data() const -> const std::uint8_t * { return _data; }
    constexpr auto dataSize() const -> const std::size_t & { return _dataSize; }

private:
    AddressInfo     _addrInfo;
    std::uint8_t    _data[MAX_PACKET_SIZE];
    std::size_t     _dataSize;
};

} // namespace Firework::Networking
