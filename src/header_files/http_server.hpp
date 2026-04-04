#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

// RFC 2616 §8.1.4 — servers SHOULD limit simultaneous persistent connections.
static constexpr int MAX_CONNECTIONS = 128;

// Idle keep-alive timeout in seconds before the server closes a silent connection.
static constexpr int KEEPALIVE_TIMEOUT_SEC = 5;

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

    // RFC 2616 §8.1.4 — count active connections to enforce MAX_CONNECTIONS
    std::atomic<int> activeConnections{0};
    std::mutex connectionMutex;

    // client and server helpers
    std::string getServerIP(int client_socket);
    std::string getClientIP(int client_socket);

    // request & response
    Router router;
    Request httpRequest;
    Response httpResponse;

    // map to store method, route, and http version
    // void extractHttpHeader(std::vector<HttpHeader> &headerVector, const std::string &message);

    void acceptConnections();
    void handleConnection(int client_socket);
    void handleRequest(const std::string &raw, Request &req, Response &res);
    void handleResponse(int client_socket, Response &res);
};

#endif