#include "win_udp_server.hpp"

#include <string_view>
#include <string>
#include <print>

#include "WinSock2.h"
#include "ws2tcpip.h"

namespace Firework::Internal
{
    
WinUDPServer::WinUDPServer()
    : _receiveSocket{INVALID_SOCKET}
    , _running{false}
    , _packetsQueue{}
{
}

auto WinUDPServer::create_socket(std::uint32_t port) -> bool {
    ::WSADATA wsadata;
    int result = ::WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (result != 0) {
        std::println("WSAStartup failed with error: {}", result);
        return false;
    }
 
    std::string portStr = std::to_string(port);
    
    ::PADDRINFOA resultAddrInfo = NULL;
    ::ADDRINFOA hints{0};    
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // Socket type that supports UDP
    hints.ai_protocol = ::IPPROTO_UDP;

    constexpr std::string_view host = "127.0.0.1";
    result = ::getaddrinfo(host.data(), portStr.c_str(), &hints, &resultAddrInfo);
    if (result != 0) {
        std::println("getaddrinfo failed with code {}", result);
        ::WSACleanup();
        return false;
    }

    ::SOCKET listenSocket = ::socket(
        resultAddrInfo->ai_family, 
        resultAddrInfo->ai_socktype, 
        resultAddrInfo->ai_protocol
    );

    if (listenSocket == INVALID_SOCKET) {
        std::println("Error at socket(): {}", ::WSAGetLastError());
        
        // ::freeaddrinfo(resultAddrInfo);
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
        std::println("Bind failed: {}", ::WSAGetLastError());
    
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
            reinterpret_cast<PSOCKADDR>(&clientAddr), &clientAddrLen
        );
        
        if (recvSize == SOCKET_ERROR) {
            std::println("recv error: {}", ::WSAGetLastError());
            if (!_running.load()) break;
            continue;
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::uint16_t port = ntohs(clientAddr.sin_port);

        AddressInfo info{};
        info.ipAddr = std::string(clientIP);
        info.port = port;

        UDPPacket packet{};
        packet.addrInfo = info;
        std::memcpy(packet.data, recvBuf, recvSize);
        packet.dataSize = recvSize;

        {
            std::lock_guard<std::mutex> guard(_packetsMutex);
            _packetsQueue.push(packet);
        }
    }
}

} // namespace Firework::Internal

