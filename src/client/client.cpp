#include <iostream>
#include <chrono>
#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#include "b_tree.h"
#include <server_protocol.h>
#include "murmurhash.h"

// Bart DB default port
#define DEFAULT_PORT 6122
#define DEFAULT_IP "127.0.0.1"

#define SIMULATE_NETWORK_LATENCY 1

void print_help() {
    std::cout << "Usage: ./client [--hosts or -h <ip1>:<port1>,<ip2>:<port2>...] [--replicationFactor or -rp <factor>]" << std::endl;
}

// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
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


int make_connection(const char* host, int port) {
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
    return client_fd;
}

std::vector<int>* rankServers(std::vector<std::string> hosts, key_type key) {
    std::vector<int>* ranks = new std::vector<int>();
    std::vector<unsigned int> scores = std::vector<unsigned int>();
    for (size_t i = 0; i < hosts.size(); i++)
    {
        int bytes = sizeof(key) + (sizeof(char) * hosts[i].size());
        unsigned char* data = (unsigned char*)malloc(bytes);
        memcpy(data, &key, sizeof(key));
        memcpy(data + sizeof(key), hosts[i].begin().base(), (sizeof(char) * hosts[i].size()));
        unsigned int score = murmur3_32(data, bytes, 42);
        scores.push_back(score);
        free(data);
    }

    std::vector<int> availableRanks = {};
    for (size_t i = 0; i < hosts.size(); i++)
    {
        availableRanks.push_back(i);
    }

    for (size_t i = 0; i < hosts.size(); i++)
    {
        // Find min element
        auto min_it = std::min_element(scores.begin(), scores.end());

        // Get index
        int index = std::distance(scores.begin(), min_it);
        ranks->push_back(availableRanks[index]);
        availableRanks.erase(availableRanks.begin() + index);
        scores.erase(scores.begin() + index);
    }

    return ranks;
}

ServerPacket* send_distributed_request(ServerPacket* request, std::vector<int> client_fds, std::vector<std::string> hosts, int replicationFactor) {
    ServerPacket* response;
    std::vector<int>* serverRanks = rankServers(hosts, request->key);
    

    if(strcmp(request->header.c_str(), "PUT") == 0) {
        // Make the request to all top servers
        for (size_t i = 0; i < replicationFactor; i++)
        {
            // std::cout << "Sending info to server " << hosts[serverRanks->at(i)] << std::endl;
            response = get_response(client_fds[serverRanks->at(i)], request);
        }
    }
    else if((strcmp(request->header.c_str(), "GET") == 0) || (strcmp(request->header.c_str(), "CONTAINS") == 0)) {
        // Make the request to a random server that contains the replica
        int serverIndex = rand() % replicationFactor;
        // std::cout << "Getting info about from server " << hosts[serverRanks->at(serverIndex)] << std::endl;
        response = get_response(client_fds[serverRanks->at(serverIndex)], request);
    }
    else {
        // Shut down command
        response = get_response(client_fds[0], request);
    }

    delete serverRanks;
    return response;
}


int main(int argc, char** argv) {
    int replicationFactor = 1;
    std::vector<std::string> servers = {std::string(DEFAULT_IP) + ":" + std::to_string(DEFAULT_PORT)};
    std::vector<const char*> hosts = {DEFAULT_IP};
    std::vector<int> ports = {DEFAULT_PORT};

    if(argc <= 2) {
        print_help();
        exit(1);
    }

    bool runStressTest = false;
    bool readStressTest = false;
    int iterations = 0;

    // Parse CLI args
    for (size_t arg = 1; arg < argc; arg++) {
        if(arg == 1) {
            if(strcmp(argv[arg], "STRESS_READ") == 0) {
                runStressTest = true;
                readStressTest = true;
                continue;
            }
        }
        if(arg == 1) {
            if(strcmp(argv[arg], "STRESS_WRITE") == 0) {
                runStressTest = true;
                readStressTest = false;
                continue;
            }
        }
        if(arg == 2 && runStressTest) {
            iterations = atoi(argv[arg]);
            std::cout << "Running for " << ((iterations == 0) ? "infinite" : std::to_string(iterations)) << " iterations." << std::endl; 
            continue;
        }
        if(strcmp(argv[arg], "--hosts") == 0 || strcmp(argv[arg], "-h") == 0) {
            servers.clear();
            hosts.clear();
            ports.clear();

            if(arg + 1 < argc) {

                // Split on comma
                std::string argument = std::string(argv[arg+1]);
                std::vector<std::string> ips = split(argument, ",");
                for (size_t i = 0; i < ips.size(); i++)
                {
                    if(ips[i] == "") {
                        continue;
                    }
                    std::string serverIp = ips[i];
                    
                    // Split on colon
                    size_t colon_pos = serverIp.find(':');
                    std::string ip = serverIp.substr(0, colon_pos);
                    int port = atoi(serverIp.substr(colon_pos+1).c_str());

                    hosts.push_back(ip.c_str());
                    ports.push_back(port);
                    servers.push_back(std::string(ip) + ":" + std::to_string(port));
                }
                arg+=1;
                continue;
            }
            else {
                print_help();
            }
            arg += 2;
        } if(strcmp(argv[arg], "--replicationFactor") == 0 || strcmp(argv[arg], "-rp") == 0) {
            if(arg + 1 < argc) {
                int value = atoi(argv[arg+1]);
                if(value <= 0) {
                    std::cout << "Replication factor must be > 0" << std::endl;
                    print_help();
                    exit(1);
                }
                replicationFactor = value;
            }
            else {
                std::cout << "No value provided" << std::endl;
                print_help();
                exit(1);
            }
            arg++;
        } else {
            std::cout << "Uknown argument " << argv[arg] << std::endl;
            print_help();
            exit(1);
        }
    }

    // std::cout << "Using port " << port << std::endl;
    // std::cout << "Using host " << host << std::endl;

    std::vector<int> client_fds = std::vector<int>();
    for (size_t i = 0; i < hosts.size(); i++)
    {
        client_fds.push_back(make_connection(hosts[i], ports[i]));
    } 

    // The way this client is built it is limited to only sending one word (no space) keys and values
    if(runStressTest) {
        
        // Timing code brrowed from https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        auto t1 = high_resolution_clock::now();

        int requestCount = 0;
        for (size_t i = 0; (i < iterations) || iterations == 0; i++)
        {
            if(SIMULATE_NETWORK_LATENCY) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            ServerPacket* packet = new ServerPacket();
            packet->header = readStressTest ? "GET" : "PUT";
            packet->key = generate_random_string();
            packet->value = generate_random_string();

            ServerPacket* response = send_distributed_request(packet, client_fds, servers, replicationFactor);

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
                std::cout << "Exiting..." << std::endl;
                break;
            } else if(command == "SHUTDOWN") {
                std::cout << "Shutting down server..." << std::endl;
                packet->header = "SHUTDOWN";
                ServerPacket* response = send_distributed_request(packet, client_fds, servers, replicationFactor);
                std::cout << "Server was shut down gracefully." << std::endl;
                delete response;
                return 0;
            } else if(command == "CONTAINS") {
                getline( iss, key, ' ' );
                packet->header = "CONTAINS";
                packet->key = key.c_str();
                ServerPacket* response = send_distributed_request(packet, client_fds, servers, replicationFactor);
                std::cout << response->header.c_str() << std::endl;
                delete response;

            } else if(command == "GET") {
                getline( iss, key, ' ' );
                packet->header = "GET";
                packet->key = key.c_str();
                ServerPacket* response = send_distributed_request(packet, client_fds, servers, replicationFactor);
                std::cout << response->header.c_str() << std::endl;
                std::cout << response->value.c_str() << std::endl;
                delete response;

            } else if(command == "PUT") {
                getline( iss, key, ' ' );
                getline( iss, value, ' ' );
                packet->header = "PUT";
                packet->key = key.c_str();
                packet->value = value.c_str();
                ServerPacket* response = send_distributed_request(packet, client_fds, servers, replicationFactor);
                std::cout << response->header.c_str() << std::endl;
                delete response;

            } else {
                std::cout << "Unknown command: "  << command << std::endl;
                std::cout << "Supported Commands:" << std::endl;
                std::cout << " - CONTAINS <key>" << std::endl;
                std::cout << " - GET <key>" << std::endl;
                std::cout << " - PUT <key> <value>" << std::endl;
                std::cout << " - SHUTDOWN" << std::endl;
                std::cout << " - EXIT" << std::endl;
            }

            delete packet;
        }
    }
}