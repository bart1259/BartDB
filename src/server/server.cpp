#include <iostream>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>

#include <vector>
#include <algorithm>

#include "server_protocol.h"
#include "b_tree.h"
#include "database.h"

// Bart DB default port
#define DEFAULT_PORT 6122
#define MAX_PENDING 32

#define USE_MULTIPLEXING 1

void print_help() {
    std::cout << "Usage: ./server <db file> [--port or -p <port_number>]" << std::endl;
}

BTree<31>* btree;

ServerPacket* handle_request(ServerPacket* request) {
    ServerPacket* response = new ServerPacket();
    if(strcmp(request->header.c_str(), "CONTAINS") == 0) {
        value_type v;
        // std::cout << " - contains " << request->key << std::endl;
        bool contains = btree->get(request->key.c_str(), &v);
        response->header = contains ? "TRUE" : "FALSE";
        response->key = request->key;
    } else if (strcmp(request->header.c_str(), "GET") == 0) {
        value_type v;
        bool contains = btree->get(request->key.c_str(), &v);
        response->header = contains ? "OK" : "ERROR";
        response->key = request->key;
        if(contains) {
            response->value = v;
        }
        // std::cout << " - get " << request->key << " (" << request->value << ")" << std::endl;
    } else if (strcmp(request->header.c_str(), "PUT") == 0) {
        // std::cout << " - put " << request->key << " " << request->value << std::endl;
        btree->put(request->key.c_str(), request->value.c_str());
        response->header = "OK";
        response->key = request->key;
        response->value = request->value;
    } else if (strcmp(request->header.c_str(), "SHUTDOWN") == 0) {
        // std::cout << " - shutdown requested " << std::endl;
        response->header = "OK";
    }  else {
        response->header = "ERROR";
    }
    return response;
}

enum client_action {
    keep_alive,
    client_disconnect,
    kill_all_clients
};

client_action handle_client(int client_fd) {
    char buffer[PACKET_SIZE];

    int total_bytes_recevied = 0;
    while(total_bytes_recevied < PACKET_SIZE) {
        int bytes_read = recv(client_fd, &(buffer[total_bytes_recevied]), PACKET_SIZE - total_bytes_recevied, 0);
        if (bytes_read < 0) {
            std::cout << "Read failed" << std::endl;
            return client_action::client_disconnect;
        }
        if (bytes_read == 0) {
            std::cout << "Connection closed" << std::endl;
            return client_action::client_disconnect;
        }
        total_bytes_recevied += bytes_read;
    }

    ServerPacket* request = packet_from_bytes(buffer);

    // For sanity clear packet_data
    std::fill(&(buffer[0]), &(buffer[PACKET_SIZE]), 0);

    if(strcmp(request->header.c_str(), "SHUTDOWN") == 0) {
        // Do not send response
        return client_action::kill_all_clients;
    }

    ServerPacket* response = handle_request(request);

    packet_to_bytes(response, buffer);

    ssize_t bytes_sent = send(client_fd, buffer, PACKET_SIZE, 0);
    if (bytes_sent < 0) {
        std::cout << "Failed to send data to client" << std::endl;
        exit(1);
    }

    if (bytes_sent != PACKET_SIZE) {
        std::cout << "Sent unexpected number of bytes" << std::endl;
        exit(1);
    }  
    return client_action::keep_alive;
}


int main(int argc, char** argv) {
    std::cout << "Starting BartDB Server..." << std::endl;

    int port = DEFAULT_PORT;

    if (argc <= 1) {
        print_help();
    }

    char* db_file_name = argv[1];
    std::cout << "Opening " << db_file_name << ".bartdb" << std::endl;

    Database<31>* db = new Database<31>(db_file_name);
    btree = new BTree<31>(db);
    db->open();

    // Parse CLI args
    for (size_t arg = 2; arg < argc; arg++) {
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
        }
    }

    std::cout << "Using port " << port << std::endl;

    // Code largely inspured by "TCP/IP Sockets in C, Second edition"
    // Creaste server socket file descriptior
    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_fd < 0) {
        std::cout << "Failed to create socket, aborting..." << std::endl;
        exit(1);
    }

    // Construct local address structure
    struct sockaddr_in serv_addr; // Local address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Failed to bind to (0.0.0.0:" << port << "). Is there another service using it?" << std::endl;
        exit(1);
    }

    // Start listening
    if (listen(server_fd, MAX_PENDING) < 0) {
        std::cout << "Failed to listen on socket." << std::endl;
        exit(1);
    }
    
    std::cout << "Listening..." << std::endl;

#if USE_MULTIPLEXING
    std::cout << "Using Multiplexing" << std::endl;
    std::vector<int> connected_clients = std::vector<int>();
    connected_clients.clear();
    while (true)
    {
        fd_set read_sockets;
        FD_ZERO(&read_sockets);
        FD_SET(server_fd, &read_sockets);

        for (size_t i = 0; i < connected_clients.size(); i++)
        {
            FD_SET(connected_clients[i], &read_sockets);
        }

        int max_fd = 0;        
        if (connected_clients.size() > 0)
        {
            max_fd = *std::max_element(connected_clients.begin(), connected_clients.end())+1;
        }
        max_fd = std::max(max_fd, server_fd) + 1;

        
        int error = select(max_fd, &read_sockets, NULL, NULL, NULL);
        if (error <= 0) {
            std::cout << "Error selecting file descriptiors" << std::endl;
            exit(1);
        }

        if(FD_ISSET(server_fd, &read_sockets)) {
            struct sockaddr_in clntAddr;
            socklen_t clntAddrLen = sizeof(clntAddr);

            // Wait for a client to connect
            int client_fd = accept(server_fd, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (client_fd < 0) {
                std::cout << "Failed to accept connection." << std::endl;
                exit(1);
            }

            // clntSock is connected to a client!
            char clntName[INET_ADDRSTRLEN]; // String to contain client address
            if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL) {
                std::cout << "New client connected: " <<  clntName << ":" << ntohs(clntAddr.sin_port) << std::endl;
            }

            connected_clients.push_back(client_fd);
        } else {
            for (int i = connected_clients.size()-1; i >= 0; i--)
            {
                if (FD_ISSET(connected_clients[i], &read_sockets)) {
                    client_action action = handle_client(connected_clients[i]);
                    if(action == client_action::client_disconnect) {
                        // Connection closed, remove connection fd
                        connected_clients.erase(connected_clients.begin() + i);
                    }
                    if(action == client_action::kill_all_clients) {
                        // Send kill signal to all other connections
                        for (int j = 0; j < connected_clients.size(); j++)
                        {
                            if(i != j) {
                                if(close(connected_clients[j]) < 0) {
                                    std::cout << "Error killing client connection" << std::endl;
                                }
                            }
                        }

                        // Send response of succesfully shutdown
                        char buffer[PACKET_SIZE];
                        ServerPacket* response = new ServerPacket();
                        response->header = "OK";
                        packet_to_bytes(response, buffer);
                        delete response;

                        ssize_t bytes_sent = send(connected_clients[i], buffer, PACKET_SIZE, 0);
                        if (bytes_sent < 0) {
                            std::cout << "Failed to send data to client" << std::endl;
                            exit(1);
                        }

                        if (bytes_sent != PACKET_SIZE) {
                            std::cout << "Sent unexpected number of bytes" << std::endl;
                            exit(1);
                        }  
                        goto serverShutDown;
                    }
                }
            }
            
        }
    }
    

#else
    while(1) {
        struct sockaddr_in clntAddr;
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Wait for a client to connect
        int client_fd = accept(server_fd, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (client_fd < 0) {
            std::cout << "Failed to accept connection." << std::endl;
            exit(1);
        }

        // clntSock is connected to a client!
        char clntName[INET_ADDRSTRLEN]; // String to contain client address
        if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL) {
            std::cout << "New client connected: " <<  clntName << ":" << ntohs(clntAddr.sin_port) << std::endl;

        }

        while(1) {
            if(handle_client(client_fd) == false) {
                break;
            }
        }

    }
#endif

serverShutDown:

    std::cout << "Server was shut down gracefully" << std::endl;
    close(server_fd);
    db->close();
}