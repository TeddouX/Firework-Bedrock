#pragma once

namespace Firework::Networking::BP
{
  
// Stores information about a client
struct BedrockClient {
    // True after completing the handshake
    bool loginSuccess;
    // True after sending all ressource pack data
    bool playing;
    // True after Network Settings packet was sent
    bool connectionCompressed;
    // True after Client To Server Handshake packet was received
    bool connectionEncrypted;
};

} // namespace Firework::Networking::BP
