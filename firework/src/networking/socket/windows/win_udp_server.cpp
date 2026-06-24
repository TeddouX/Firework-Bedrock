#include "win_udp_server.hpp"

#include <string_view>
#include <string>
#include <print>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <optional>

#include "../../utils/byte.hpp"
#include "../../../core/logger.hpp"

namespace Firework::Networking::Socket
{

constexpr const std::uint8_t LOCAL_IP_BYTES[] = {127, 0, 0, 1};

WinUDPServer::WinUDPServer()
    : _receiveSocket{INVALID_SOCKET}
    , _running{false}
    , _packetsQueue{}
{}

auto WinUDPServer::create_socket(std::uint32_t port) -> bool {
    ::WSADATA wsadata;
    int result = ::WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (result != 0) {
        LOGGER.critical("WSAStartup failed with error: {}", result);
        return false;
    }
 
    std::string portStr = std::to_string(port);
    
    ::PADDRINFOA resultAddrInfo = NULL;
    ::ADDRINFOA hints{0};    
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // Socket type that supports UDP
    hints.ai_protocol = ::IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // constexpr std::string_view host = "192.168.1.13";
    result = ::getaddrinfo(nullptr, portStr.c_str(), &hints, &resultAddrInfo);
    if (result != 0) {
        LOGGER.critical("getaddrinfo failed with code {}", result);
        ::WSACleanup();
        return false;
    }

    ::SOCKET listenSocket = ::socket(AF_INET, SOCK_DGRAM, ::IPPROTO_UDP);
    if (listenSocket == INVALID_SOCKET) {
        LOGGER.critical("Error at socket(): {}", ::WSAGetLastError());
        
        ::freeaddrinfo(resultAddrInfo);
        ::WSACleanup();

        return false;
    }
    
    _receiveSocket = listenSocket;

    result = ::bind(
        _receiveSocket, 
        resultAddrInfo->ai_addr, 
        static_cast<int>(resultAddrInfo->ai_addrlen)
    );

    if (result == SOCKET_ERROR) {
        LOGGER.critical("Bind failed: {}", ::WSAGetLastError());
    
        ::freeaddrinfo(resultAddrInfo);
        ::closesocket(_receiveSocket);
        ::WSACleanup();
    
        return false;
    }

    ::freeaddrinfo(resultAddrInfo);

    return true;
}

auto WinUDPServer::start() -> void {
    _running.store(true);

    // Start the receive thread
    _receiveThread = std::thread{&WinUDPServer::receive_thread, this};
}

auto WinUDPServer::stop() -> void {
    _running.store(false);
    ::closesocket(_receiveSocket);
    _receiveThread.join();
}

auto WinUDPServer::try_pop_packet(UDPPacket &outPacket) -> bool {
    std::lock_guard<std::mutex> guard(_packetsMutex);

    if (_packetsQueue.empty())
        return false;

    outPacket = std::move(_packetsQueue.front());
    _packetsQueue.pop();

    return true;
}

auto WinUDPServer::receive_thread() -> void {
    char recvBuf[MAX_PACKET_SIZE];
    ::SOCKADDR_IN clientAddr;

    while (_running.load()) {
        int clientAddrLen = sizeof(clientAddr);
        int recvSize = ::recvfrom(
            _receiveSocket, 
            recvBuf, sizeof(recvBuf), 
            0,
            reinterpret_cast<::PSOCKADDR>(&clientAddr), &clientAddrLen
        );
        
        if (recvSize == SOCKET_ERROR) {
            LOGGER.error("recv error: {}", ::WSAGetLastError());
            if (!_running.load()) break;
            continue;
        }

        ::PIN_ADDR addrData = &clientAddr.sin_addr;
        // Ignore localhost requests
        if (std::memcmp(addrData, LOCAL_IP_BYTES, 4) == 0)
            continue;

        char clientIP[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, addrData, clientIP, sizeof(clientIP));
        std::uint16_t port = network_to_host(clientAddr.sin_port);

        std::vector<std::uint8_t> data{recvBuf, recvBuf + recvSize};
        Address addrInfo{std::string(clientIP), port, AddressFamily::IPv4};
        UDPPacket packet{addrInfo, data};

        {
            std::lock_guard<std::mutex> guard(_packetsMutex);
            _packetsQueue.push(packet);
        }
    }
}

auto WinUDPServer::send(const std::vector<std::uint8_t> &data, const Address &addrInfo) -> bool {
    auto dataPtr = reinterpret_cast<const char *>(data.data());
    const size_t dataSize = data.size();

    SOCKADDR_IN addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = host_to_network(addrInfo.port());
    
    int result = ::inet_pton(AF_INET, addrInfo.ipAddr().data(), &addr.sin_addr);
    if (result != 1) {
        LOGGER.error("inet_pton failed for ip='{}', result={}", addrInfo.ipAddr(), result);
        return false;
    }

    result = ::sendto(
        _receiveSocket,
        dataPtr, dataSize,
        0,
        reinterpret_cast<const PSOCKADDR>(&addr), sizeof(addr)
    );

    if (result == SOCKET_ERROR) {
        LOGGER.error("sendto failed: {}", ::WSAGetLastError());
        return false;
    }
    
    return true;
}

auto WinUDPServer::send_all(const std::vector<std::vector<std::uint8_t>> &packets, const Address &addrInfo) -> bool {
    for (const std::vector<std::uint8_t> &packet : packets) {
        if (!send(packet, addrInfo))
            return false;
    }
    
    return true;
}


} // namespace Firework::Networking::Socket
