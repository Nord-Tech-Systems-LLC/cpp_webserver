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
    void get(const std::string &route, std::function<void(Request &, Response &)> handler);
    void post(const std::string &route, std::function<void(Request &, Response &)> handler);
    void put(const std::string &route, std::function<void(Request &, Response &)> handler);
    void del(const std::string &route, std::function<void(Request &, Response &)> handler);
    void use(std::function<void(Request &, Response &)> handler);
    void use(const std::string &route, std::function<void(Request &, Response &)> handler);
    void middleware(const std::map<std::string, std::string> &headers);

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
    Router router;
    Request httpRequest;
    Response httpResponse;

    // map to store method, route, and http version
    // void extractHttpHeader(std::vector<HttpHeader> &headerVector, const std::string &message);
};

#endif