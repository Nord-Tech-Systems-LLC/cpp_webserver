#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <cstring>
#include <map>
#include <functional>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iostream>

#ifndef MAX_HTTP_HEADERS
#define MAX_HTTP_HEADERS 30
#endif

struct HttpHeader
{
    std::string name;  // Header name
    std::string value; // Header value
};

struct HttpMessage
{
    std::string method, uri, query, proto; // Request & response
    HttpHeader headers[MAX_HTTP_HEADERS];  // Headers
    std::string body;                      // Body
    std::string head;                      // Request + headers
    std::string message;                   // Request + headers + body
};

class HttpServer
{
public:
    HttpServer(const char *ip_address, const char *port);
    ~HttpServer();
    void start();
    void printRoutes();

    // dispatch events
    void addRoute(const std::string &path, std::function<void(Request &, Response &)> handler);

private:
    const char *port;
    const char *ip_address;
    int server_socket;

    // helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    void handleRequest(int client_socket);
    void handleResponse(int client_socket);
    void acceptConnections();

    // request & response
    Request httpRequest;
    Response httpResponse;
    bool checkRoutes();
    std::map<std::string, std::function<void(Request &, Response &)>> routes;

    // map to store method, route, and http version
    const std::string getHttpHeader(struct HttpMessage &message, const std::string &name);
    void parseHttpRequest(const std::string &requestBuffer);
};

#endif