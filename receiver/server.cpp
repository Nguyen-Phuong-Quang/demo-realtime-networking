// server.cpp
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

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
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    if (listen(sockfd, 1) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (newsockfd < 0) {
        std::cerr << "Error accepting connection" << std::endl;
        return 1;
    }

    // Set bandwidth limit for the socket
    int bandwidth_limit = BANDWIDTH_LIMIT;
    setsockopt(newsockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &bandwidth_limit, sizeof(bandwidth_limit));

    char buffer[1024];
    ssize_t bytes_received;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(newsockfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        // Calculate checksum of received data
        unsigned short receivedChecksum = 0;
        memcpy(&receivedChecksum, buffer + bytes_received - sizeof(unsigned short), sizeof(unsigned short));

        // Remove checksum from data for calculation
        bytes_received -= sizeof(unsigned short);

        // Calculate checksum of received data
        unsigned short calculatedChecksum = calculateChecksum(buffer, bytes_received);

        // Compare checksums
        if (receivedChecksum == calculatedChecksum) {
            std::cout << "Received data: " << buffer << std::endl;
            std::cout << "Checksum is valid." << std::endl;
        } else {
            std::cout << "Received data: " << buffer << std::endl;
            std::cerr << "Checksum is NOT valid." << std::endl;
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
