#include "server_protocol.h"

ServerPacket* packet_from_bytes(char* bytes) {
    ServerPacket* ret = new ServerPacket();
    std::copy(
        bytes,
        bytes + 64,
        reinterpret_cast<char*>(&ret->header)
    );
    std::copy(
        bytes + 64,
        bytes + 128,
        reinterpret_cast<char*>(&ret->key)
    );
    std::copy(
        bytes + 128,
        bytes + 192,
        reinterpret_cast<char*>(&ret->value)
    );
    return ret;
}


void packet_to_bytes(ServerPacket* packet, char* bytes) {
    std::copy(
        reinterpret_cast<const char*>(&packet->header),
        reinterpret_cast<const char*>(&packet->header) + 64,
        bytes
    );
    std::copy(
        reinterpret_cast<const char*>(&packet->key),
        reinterpret_cast<const char*>(&packet->key) + 64,
        (bytes + 64)
    );
    std::copy(
        reinterpret_cast<const char*>(&packet->value),
        reinterpret_cast<const char*>(&packet->value) + 64,
        (bytes + 128)
    );
    std::fill(bytes + 192, bytes + 256, (char)0);
}