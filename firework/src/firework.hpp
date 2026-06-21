#pragma once
#include <cstdint>

#define BEDROCK_PROTOCOL_VERSION 1001
#define BEDROCK_VERSION_NAME "1.26.31"

#if defined(_WIN32)
    #define FIREWORK_WINDOWS
#elif defined(__linux__)
    #error "Linux not implemented at this time"
    #define FIREWORK_LINUX
#endif

#ifdef NDEBUG
    #define FIREWORK_RELEASE
#endif
