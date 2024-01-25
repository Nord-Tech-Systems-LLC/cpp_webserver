#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <cstring>

#include "route_handler.hpp"

class HttpServer {
   public:
    HttpServer(const char *port);
    // ~HttpServer();
    void start();
    void addRoute(const std::string &path, std::function<void(int)> handler);
    void sendCustomResponse(int client_socket, const char *response);

   private:
    const char *port;
    int server_socket;
    RouteHandler routeHandler;

    // Helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    void handleRequest(int client_socket);
    void acceptConnections();
    void sendHttpGetResponse(int client_socket);

    // request / response
    // map to store method, route, and http version
    std::map<std::string, std::string> parsedInfo;
    std::map<std::string, std::string> parseHttpRequest(const std::string &requestBuffer);
};

#endif