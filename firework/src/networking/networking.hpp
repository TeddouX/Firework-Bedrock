#pragma once
#include "../core/logger.hpp"

namespace Firework::Networking
{

#define RAKNET_PROTOCOL_VERSION 11

// https://github.com/CloudburstMC/Nukkit/blob/master/src/main/java/cn/nukkit/network/encryption/EncryptionUtils.java
constexpr std::string_view MOJANG_PUBLIC_KEY_BASE64 = 
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAECRXueJeTDqNRRgJi/vlRufBy"
"u/2G0i2Ebt6YMar5QX/R0DIIyrJMcUpruK4QveTfJSTp3Shlq4Gk34cD"
"/4GUWwkv0DVuzeuB+tXija7HBxii03NHDbPAD0AKnLr2wdAp";

// Maximum packet size racknet will send
constexpr std::size_t MAX_PACKET_SIZE = 1492zu;

inline static Firework::Logger LOGGER{"Firework", "Networking"};

} // namespace Firework::Networking

