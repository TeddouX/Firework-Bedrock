cmake_minimum_required(VERSION 4.1)
project(FireworkSource VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(FireworkSource)
target_sources(FireworkSource PRIVATE
    "binary/binary_reader.cpp"
    "binary/binary_reader.hpp"
    "binary/binary_writer.cpp"
    "binary/binary_writer.hpp"
    
    "core/logger.cpp"
    "core/logger.hpp"

    "networking/raknet/raknet_connection.cpp"
    "networking/raknet/raknet_connection.hpp"
    "networking/raknet/raknet_frame_set_packet.cpp"
    "networking/raknet/raknet_frame_set_packet.hpp"
    "networking/raknet/raknet_packets.cpp"
    "networking/raknet/raknet_packets.hpp"
    "networking/raknet/raknet_server.cpp"
    "networking/raknet/raknet_server.hpp"
    "networking/utils/byte.hpp"
    "networking/utils/ip.cpp"
    "networking/utils/ip.hpp"
    "networking/address.cpp"
    "networking/address.hpp"
    "networking/udp_packet.hpp"
    "networking/udp_server.hpp"
    "networking/uint24.hpp"

    "firework.hpp"
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
