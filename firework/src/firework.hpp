#pragma once
#include <cstdint>

#include "networking/uint24.hpp"

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

namespace Firework 
{

template <typename _Ty>
concept IsIntegral = requires { std::is_integral_v<_Ty> | std::is_same_v<_Ty, uint24_t>; };

} // namespace Firework