cmake_minimum_required(VERSION 4.1)
message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")
project(FireworkServer VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(nlohmann_json  CONFIG REQUIRED)
find_package(OpenSSL               REQUIRED)
find_package(ZLIB                  REQUIRED)


add_executable(FireworkServer)
target_sources(FireworkServer PRIVATE
    "binary/binary_reader.cpp"
    "binary/binary_reader.hpp"
    "binary/binary_writer.cpp"
    "binary/binary_writer.hpp"
    
    "core/codec/codec.hpp"
    "core/codec/object_codec.hpp"
    "core/logger.cpp"
    "core/logger.hpp"

    "networking/bedrock_protocol/bedrock_protocol.hpp"
    "networking/bedrock_protocol/bp_decryptor.cpp"
    "networking/bedrock_protocol/bp_decryptor.hpp"
    "networking/raknet/raknet_connection.cpp"
    "networking/raknet/raknet_connection.hpp"
    "networking/raknet/raknet_frame_set_packet.cpp"
    "networking/raknet/raknet_frame_set_packet.hpp"
    "networking/raknet/raknet_packets.cpp"
    "networking/raknet/raknet_packets.hpp"
    "networking/raknet/raknet_server.cpp"
    "networking/raknet/raknet_server.hpp"
    "networking/socket/udp_packet.hpp"
    "networking/socket/udp_server.hpp"
    "networking/utils/byte.hpp"
    "networking/utils/ip.cpp"
    "networking/utils/ip.hpp"
    "networking/address.cpp"
    "networking/address.hpp"
    "networking/uint24.hpp"

    "firework.hpp"
    "main.cpp"
)

target_link_libraries(FireworkServer PRIVATE
    OpenSSL::Crypto
    ZLIB::ZLIB
    nlohmann_json::nlohmann_json
)

if (WIN32)
    target_sources(FireworkServer PRIVATE
        "networking/socket/windows/win_udp_server.hpp"
        "networking/socket/windows/win_udp_server.cpp"
    )

    target_link_libraries(FireworkServer PRIVATE 
        wsock32 
        ws2_32
    )
elseif (UNIX AND NOT APPLE)
    target_sources(FireworkServer PRIVATE
        
    )
elseif (APPLE)
    message(FATAL_ERROR "Apple is not supported by this project, and there are no plans of supporting it.")
endif()
