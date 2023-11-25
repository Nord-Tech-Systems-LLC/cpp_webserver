#pragma once
#ifndef INCLUDED_HTTP_TCPSERVER_LINUX
#define INCLUDED_HTTP_TCPSERVER_LINUX

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <string>

namespace http {

class TcpServer {
   public:
    TcpServer(std::string ip_address, int port);
    ~TcpServer();
    bool startListen();
    void closeServer();

   private:
    std::string m_ip_address;
    int m_port;
    int m_socket;
    int m_new_socket;
    long m_incomingMessage;
    struct sockaddr_in m_socketAddress;
    unsigned int m_socketAddress_len;
    std::string m_serverMessage;

    int startServer();

    void acceptConnection(int &new_socket);
    std::string buildResponse();
    void sendResponse();
};

}  // namespace http

#endif
