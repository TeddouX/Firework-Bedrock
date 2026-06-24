#include "bp_decryptor.hpp"

#include <zlib.h>

#include "bedrock_protocol.hpp"

namespace Firework::Networking::BP
{

auto zlib_decompress_packet(
    std::vector<std::uint8_t> &compressedPacket, 
    std::size_t maxSize // Default 1MB
) -> std::vector<std::uint8_t> {
    z_stream stream{};
    if (inflateInit(&stream) != Z_OK) {
        LOGGER.error("Failed to init zlib stream for decompression");
        return {};
    }

    std::vector<std::uint8_t> out;
    out.reserve(compressedPacket.size() * 2);

    std::uint8_t chunk[1024 * 64]{0};

    stream.next_in  = static_cast<Bytef *>(compressedPacket.data());
    stream.avail_in = static_cast<uInt>(compressedPacket.size());

    // Inflate part by part to avoid getting zip bombed
    int res;
    do {
        stream.next_out  = chunk;
        stream.avail_out = sizeof(chunk);

        // Z_NO_FLUSH: Allow a sliding window to be allocated so we can call inflate again
        res = inflate(&stream, Z_NO_FLUSH);

        if (res == Z_STREAM_ERROR ||
            res == Z_DATA_ERROR   ||
            res == Z_MEM_ERROR
        ) {
            inflateEnd(&stream);
            LOGGER.error("zlib decompression failed, error: {}", res);
            return {};
        }

        size_t got = sizeof(chunk) - stream.avail_out;
        out.insert(out.end(), chunk, chunk + got);

        if (out.size() > maxSize) {
            inflateEnd(&stream);
            LOGGER.error("Packet too large: {} bytes exceeds limit of {}",
                out.size(),
                maxSize
            );

            return {};
        }
    } while (res != Z_STREAM_END);

    inflateEnd(&stream);
    return out;
}

} // namespace Firework::Networking::BP