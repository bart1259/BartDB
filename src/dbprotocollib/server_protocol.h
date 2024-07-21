#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "fixed_string.h"
#include "b_tree.h"

#define PACKET_SIZE 256

/**
 * Incoming packet looks like
 * |  64 Bytes  |  64 Bytes  |  64 Bytes  |  64 Bytes  |
 * |   Method   |    Key     |   Value    |  Padding   |
 * 0            64          128          192
 * 
 * Method is an ASCII Encoded null terminated string which can be: 
 *    - "CONTAINS" => Check if Payload key exists in DB
 *    - "GET"      => Get key from database
 *    - "PUT"      => Put KV pair into database
 *    - "SHUTDOWN" => Shuts the server down gracefully
 * 
 * Outgoing packet looks like
 * |  64 Bytes  |  64 Bytes  |  64 Bytes  |  64 Bytes  |
 * |  Success?  |    Key     |   Value    |  Padding   |
 * 0            64          128          192
 */
struct ServerPacket
{
    FixedString<64> header;
    key_type key;
    value_type value;
};

ServerPacket* packet_from_bytes(char*);
void packet_to_bytes(ServerPacket*, char*);


#endif