#pragma once
#include <string>

#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

class UdpClient {
public:
    UdpClient();
    ~UdpClient();

    bool connect(const std::string& ip, int port);
    void disconnect();
    bool isConnected() const;

    bool send(const std::string& data);
    std::string receive(int timeout_ms = 1000);

private:
    socket_t sockfd;
    bool connected;
    std::string server_ip;
    int server_port;
};
