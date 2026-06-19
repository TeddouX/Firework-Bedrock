cmake_minimum_required(VERSION 4.1)
project(FireworkSource VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(FIREWORK_SOURCES
    # -------------------
    #      Headers
    # -------------------
    "networking/windows/win_udp_server.hpp"
    "networking/udp_server.hpp"
    
    # -------------------
    #    Source files
    # -------------------
    "networking/windows/win_udp_server.cpp"
    "main.cpp"
)

add_executable(FireworkSource ${FIREWORK_SOURCES})

if(WIN32)
    target_link_libraries(FireworkSource PUBLIC wsock32 ws2_32)
endif()
