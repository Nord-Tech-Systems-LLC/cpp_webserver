#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <cstring>

class HttpServer {
   public:
    HttpServer(const char *port) : port(port), server_socket(0){};
    ~HttpServer();
    void start();

   private:
    const char *port;
    int server_socket;

    // Helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    void handleRequest(int client_socket);
    void acceptConnections();
};

#endif