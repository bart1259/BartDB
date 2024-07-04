#include <iostream>
#include <chrono>
#include <cstring>
#include <string>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "b_tree.h"
#include <server_protocol.h>


// Bart DB default port
#define DEFAULT_PORT 6122
#define DEFAULT_IP "127.0.0.1"

void print_help() {
    std::cout << "Usage: ./client [--port or -p <port_number>] [--host or -h <ip address>]" << std::endl;
}

ServerPacket* get_response(int client_fd, ServerPacket* request) {
    char* packet_data = (char*)malloc(PACKET_SIZE);
    packet_to_bytes(request, packet_data);

    ssize_t bytes_sent = send(client_fd, packet_data, PACKET_SIZE, 0);
    if (bytes_sent < 0) {
        std::cout << "Failed to send data to client" << std::endl;
        exit(1);
    }

    if (bytes_sent != PACKET_SIZE) {
        std::cout << "Sent unexpected number of bytes" << std::endl;
        exit(1);
    }  

    // For sanity clear packet_data
    std::fill(&(packet_data[0]), &(packet_data[PACKET_SIZE]), 0);

    int total_bytes_recevied = 0;
    while(total_bytes_recevied < PACKET_SIZE) {
        int bytes_read = recv(client_fd, &(packet_data[total_bytes_recevied]), PACKET_SIZE - total_bytes_recevied, 0);
        if (bytes_read < 0) {
            std::cout << "Read failed" << std::endl;
            exit(1);
        }
        if (bytes_read == 0) {
            std::cout << "Connection closed" << std::endl;
            exit(1);
        }
        total_bytes_recevied += bytes_read;
    }

    ServerPacket* response = packet_from_bytes(packet_data);
    free(packet_data);
    return response;
}

char* generate_random_string() {
    const int SIZE = 8;
    char* strg = (char*)malloc((SIZE + 1) * sizeof(char));
    for (int i = 0; i < SIZE; i++)
    {
        strg[i] = (char)(65 + (std::rand() % 26));
    }
    strg[SIZE] = '\0';
    return strg;
}

int main(int argc, char** argv) {
    int port = DEFAULT_PORT;
    const char* host = DEFAULT_IP;

    if(argc <= 2) {
        print_help();
        exit(1);
    }

    bool runStressTest = false;

    // Parse CLI args
    for (size_t arg = 1; arg < argc; arg++) {
        if(arg == 1) {
            if(strcmp(argv[arg], "STRESS") == 0) {
                runStressTest = true;
            }
            continue;
        }
        if(strcmp(argv[arg], "--port") == 0 || strcmp(argv[arg], "-p") == 0) {
            if(arg + 1 < argc) {
                port = atoi(argv[arg+1]);
                if(port <= 0 || port >= 65536) {
                    std::cout << "Port " << argv[arg+1] << " is invalid" << std::endl;
                    print_help();
                }
            }
            else {
                print_help();
            }
            arg++;
        }
        else if(strcmp(argv[arg], "--host") == 0 || strcmp(argv[arg], "-h") == 0) {
            if(arg + 1 < argc) {
                host = argv[arg+1];
            }
            else {
                print_help();
            }
            arg++;
        } else {
            print_help();
            exit(1);
        }
    }

    std::cout << "Using port " << port << std::endl;
    std::cout << "Using host " << host << std::endl;

    // Code largely inspured by "TCP/IP Sockets in C, Second edition"
    // Creaste server socket file descriptior
    int client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(client_fd < 0) {
        std::cout << "Failed to create socket, aborting..." << std::endl;
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    int rtnVal = inet_pton(AF_INET, host, &serv_addr.sin_addr.s_addr);
    if (rtnVal <= 0) {
        std::cout << "Failed to convert IP (" << host << ") to proper strubture. Is it in the right format? (x.x.x.x)" << std::endl;
        exit(1);
    }
    serv_addr.sin_port = htons(port);

    // Establish the connection to the echo server
    if (connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection failed. Tried connecting to (" << host << ":" << port << ")" << std::endl;
        exit(1);
    }

    std::cout << "Connected to server." << std::endl;

    // The way this client is built it is limited to only sending one word (no space) keys and values
    if(runStressTest) {
        
        // Timing code brrowed from https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        auto t1 = high_resolution_clock::now();

        int requestCount = 0;
        while(true) {
            ServerPacket* packet = new ServerPacket();
            packet->header = "GET";
            packet->key = generate_random_string();
            ServerPacket* response = get_response(client_fd, packet);

            delete packet;
            delete response;

            requestCount += 1;

            auto t2 = high_resolution_clock::now();
            auto ms_int = duration_cast<milliseconds>(t2 - t1);
            if (ms_int.count() >= 1000) {
                t1 = t2;
                std::cout << requestCount << "/s" << std::endl;
                requestCount = 0;
            }
        }

    } else {
        
        std::string input;
        while(true) {
            input = "";
            std::cout << "> ";
            getline(std::cin, input);

            std::istringstream iss(input);
            std::string command;
            std::string key;
            std::string value;
            getline( iss, command, ' ' );

            ServerPacket* packet = new ServerPacket();

            if(command == "EXIT") {
                std::cout << "Exiting...." << std::endl;
                break;
            } else if(command == "CONTAINS") {
                getline( iss, key, ' ' );
                packet->header = "CONTAINS";
                packet->key = key.c_str();
                ServerPacket* response = get_response(client_fd, packet);
                std::cout << response->header.c_str() << std::endl;
                delete response;

            } else if(command == "GET") {
                getline( iss, key, ' ' );
                packet->header = "GET";
                packet->key = key.c_str();
                ServerPacket* response = get_response(client_fd, packet);
                std::cout << response->header.c_str() << std::endl;
                std::cout << response->value.c_str() << std::endl;
                delete response;

            } else if(command == "PUT") {
                getline( iss, key, ' ' );
                getline( iss, value, ' ' );
                packet->header = "PUT";
                packet->key = key.c_str();
                packet->value = value.c_str();
                ServerPacket* response = get_response(client_fd, packet);
                std::cout << response->header.c_str() << std::endl;
                delete response;

            } else {
                std::cout << "Unknown command: "  << command << std::endl;
                std::cout << "Supported Commands:" << std::endl;
                std::cout << " - CONTAINS <key>" << std::endl;
                std::cout << " - GET <key>" << std::endl;
                std::cout << " - PUT <key> <value>" << std::endl;
                std::cout << " - EXIT" << std::endl;
            }

            delete packet;
        }
    }
}