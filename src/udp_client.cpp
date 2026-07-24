#include "udp_client.h"
#include <cstring>
#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#ifndef IS_INVALID_SOCKET
#define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#define INVALID_SOCKET (-1)
#ifndef IS_INVALID_SOCKET
#define IS_INVALID_SOCKET(s) ((s) < 0)
#endif
#endif

UdpClient::UdpClient() : sockfd(INVALID_SOCKET), connected(false) {
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
    if (IS_INVALID_SOCKET(sockfd)) return false;

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
    if (!IS_INVALID_SOCKET(sockfd)) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        sockfd = INVALID_SOCKET;
    }
    connected = false;
}

bool UdpClient::isConnected() const { return connected; }

bool UdpClient::send(const std::string& data) {
    if (!connected || IS_INVALID_SOCKET(sockfd)) return false;
    struct sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &servaddr.sin_addr);
    return sendto(sockfd, data.c_str(), static_cast<int>(data.length()), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) > 0;
}

std::string UdpClient::receive(int timeout_ms) {
    if (!connected || IS_INVALID_SOCKET(sockfd)) return "";

    auto start = std::chrono::steady_clock::now();
    while (true) {
        int remaining_ms = timeout_ms;
        if (timeout_ms > 0) {
            auto now = std::chrono::steady_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            remaining_ms = timeout_ms - elapsed;
            if (remaining_ms <= 0) break;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval tv;
        tv.tv_sec = (timeout_ms > 0 ? remaining_ms : 0) / 1000;
        tv.tv_usec = ((timeout_ms > 0 ? remaining_ms : 0) % 1000) * 1000;

        int nfds = static_cast<int>(sockfd) + 1;
        int ret = select(nfds, &readfds, nullptr, nullptr, &tv);
        if (ret <= 0 || !FD_ISSET(sockfd, &readfds)) {
            break;
        }

        char buffer[4096] = {};
        struct sockaddr_in cliaddr{};
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&cliaddr, &len);
        if (n > 0) {
            buffer[n] = '\0';
            return buffer;
        }
        if (timeout_ms <= 0) break;
    }
    return "";
}
