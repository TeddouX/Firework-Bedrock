cmake_minimum_required(VERSION 4.1)
project(FireworkSource VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(FIREWORK_SOURCES
    # -------------------
    #      Headers
    # -------------------
    "networking/raknet/raknet_packets.hpp"
    "networking/raknet/raknet_server.hpp"
    "networking/utils/byte.hpp"
    "networking/utils/ip.hpp"
    "networking/windows/win_udp_server.hpp"
    "networking/address.hpp"
    "networking/udp_packet.hpp"
    "networking/udp_server.hpp"
    "firework.hpp"
    
    # -------------------
    #    Source files
    # -------------------
    "networking/raknet/raknet_packets.cpp"
    "networking/raknet/raknet_server.cpp"
    "networking/utils/ip.cpp"
    "networking/windows/win_udp_server.cpp"
    "main.cpp"
)

add_executable(FireworkSource ${FIREWORK_SOURCES})

if(WIN32)
    target_link_libraries(FireworkSource PUBLIC wsock32 ws2_32)
endif()
