// client.cpp
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <netdb.h>

#define SERVER_HOSTNAME "server_container" // Use the server container's name here
// #define SERVER_HOSTNAME "localhost"
// #define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BANDWIDTH_LIMIT 100000000  // 100Mbps in bits

// Function to calculate checksum of the data
unsigned short calculateChecksum(const char *data, size_t len) {
    unsigned int sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return static_cast<unsigned short>(sum & 0xFFFF);
}

int main() {
    int sockfd;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the server's hostname to an IP address using getaddrinfo
    int addr_status = getaddrinfo(SERVER_HOSTNAME, std::to_string(PORT).c_str(), &hints, &res);
    if (addr_status != 0) {
        std::cerr << "Error resolving hostname: " << gai_strerror(addr_status) << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memcpy(&server_addr, res->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(res);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    // Set bandwidth limit for the socket
    int bandwidth_limit = BANDWIDTH_LIMIT;
    setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &bandwidth_limit, sizeof(bandwidth_limit));

    while (true) {
        // Generate data for sending
        std::string data = "Hello, this is sample data.";
        // Calculate checksum of data
        unsigned short checksum = calculateChecksum(data.c_str(), data.length());

        // Append checksum to data
        data.append(reinterpret_cast<const char *>(&checksum), sizeof(unsigned short));

        ssize_t bytes_sent = send(sockfd, data.c_str(), data.length(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Error sending data" << std::endl;
            break;
        }

        // Wait for a while before sending next data
        usleep(1000000);
    }

    close(sockfd);
    return 0;
}
