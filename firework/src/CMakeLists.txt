cmake_minimum_required(VERSION 4.1)
project(FireworkSource VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(FireworkSource)
target_sources(FireworkSource PRIVATE
    # -------------------
    #      Headers
    # -------------------
    "networking/raknet/raknet_packets.hpp"
    "networking/raknet/raknet_server.hpp"
    "networking/utils/byte.hpp"
    "networking/utils/ip.hpp"
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
    "main.cpp"
)

if (WIN32)
    target_sources(FireworkSource PRIVATE
        "networking/windows/win_udp_server.hpp"
        "networking/windows/win_udp_server.cpp"
    )

    target_link_libraries(FireworkSource PUBLIC wsock32 ws2_32)
elseif (UNIX AND NOT APPLE)
    target_sources(FireworkSource PRIVATE
        
    )
elseif (APPLE)
    message(FATAL_ERROR "Apple is not supported by this project, and there are no plans of supporting it.")
endif()
