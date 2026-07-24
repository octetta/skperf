#include "udp_client.h"
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

UdpClient::UdpClient() : sockfd(-1), connected(false) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
}

UdpClient::~UdpClient() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool UdpClient::connect(const std::string& ip, int port) {
    disconnect();
    server_ip = ip;
    server_port = port;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return false;

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sockfd, FIONBIO, &mode);
#else
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif
    connected = true;
    return true;
}

void UdpClient::disconnect() {
    if (sockfd >= 0) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        sockfd = -1;
    }
    connected = false;
}

bool UdpClient::isConnected() const { return connected; }

bool UdpClient::send(const std::string& data) {
    if (!connected) return false;
    struct sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &servaddr.sin_addr);
    return sendto(sockfd, data.c_str(), data.length(), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) > 0;
}

std::string UdpClient::receive(int /*timeout_ms*/) {
    if (!connected) return "";
    char buffer[4096] = {};
    struct sockaddr_in cliaddr{};
    socklen_t len = sizeof(cliaddr);
    int n = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&cliaddr, &len);
    if (n > 0) {
        buffer[n] = '\0';
        return buffer;
    }
    return "";
}
