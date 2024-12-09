#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

class HttpServer {
  public:
    HttpServer(const char *ip_address, const char *port);
    ~HttpServer();
    void start();
    void printRoutes();

    // dispatch events
    void addRoute(const std::string &method,
                  const std::string &route,
                  std::function<void(Request &, Response &)> handler);

    void get(const std::string &route, std::function<void(Request &, Response &)> handler);
    void post(const std::string &route, std::function<void(Request &, Response &)> handler);
    void put(const std::string &route, std::function<void(Request &, Response &)> handler);

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

    std::string routeMethod; // GET PUT POST DELETE method per created route

    // request & response
    Request httpRequest;
    Response httpResponse;
    bool checkRoutes();
    std::map<std::string, std::function<void(Request &, Response &)>> routes;
    std::map<std::string, std::string> method_route_pair;

    // map to store method, route, and http version
    void extractHttpHeader(std::vector<HttpHeader> &headerVector, const std::string &message);
};

#endif