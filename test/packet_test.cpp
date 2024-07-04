#include <iostream>
#include <cassert>
#include <string.h>

#include "server_protocol.h"

int main() {

    std::cout << "Starting packet test..." << std::endl;

    // Create dummy packet
    char dummyPacket[] = {'C', 'O', 'N', 'T', 'A', 'I', 'N', 'S',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        'S', 'O', 'M', 'E', ' ', 'K', 'E', 'Y',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        'S', 'O', 'M', 'E', ' ', 'V', 'A', 'L',
        'U', 'E', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

    assert(sizeof(dummyPacket) == PACKET_SIZE);

    ServerPacket* parsedPacket = packet_from_bytes(dummyPacket);
    std::cout << "Header of dummy packet is " << parsedPacket->header << std::endl;
    std::cout << "Key    of dummy packet is " << parsedPacket->key << std::endl;
    std::cout << "Value  of dummy packet is " << parsedPacket->value << std::endl;

    assert(strcmp(parsedPacket->header.c_str(), "CONTAINS") == 0);
    assert(strcmp(parsedPacket->key.c_str()    , "SOME KEY") == 0);
    assert(strcmp(parsedPacket->value.c_str()  , "SOME VALUE") == 0);

    char* newPacket = (char*)malloc(PACKET_SIZE);
    packet_to_bytes(parsedPacket, newPacket);

    for (size_t i = 0; i < PACKET_SIZE; i++)
    {
        assert(newPacket[i] == dummyPacket[i]);
    }
    

    std::cout << "Done!" << std::endl;
}