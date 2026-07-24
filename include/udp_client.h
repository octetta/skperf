#pragma once
#include <string>

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
    int sockfd;
    bool connected;
    std::string server_ip;
    int server_port;
};
