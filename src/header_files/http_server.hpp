#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "cpp_webserver_include/core.hpp"

#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
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
    void start(std::function<void()> onReady = nullptr);

    // dispatch events
    void get(const std::string &path, Handler h) { router_.get(path, h); }
    void post(const std::string &path, Handler h) { router_.post(path, h); }
    void put(const std::string &path, Handler h) { router_.put(path, h); }
    void del(const std::string &path, Handler h) { router_.del(path, h); }
    void printRoutes() { router_.printRoutes(); }

  private:
    Router router_;
    const char *port;
    const char *ip_address;
    int server_socket;

    bool createSocket();
    bool bindSocket();
    bool listenSocket();

    // RFC 2616 §8.1.4 — count active connections to enforce MAX_CONNECTIONS
    std::atomic<int> activeConnections{0};
    std::mutex connectionMutex;

    void acceptConnections();
    void handleConnection(int client_socket);
    void handleRequest(const std::string &raw, Request &req, Response &res);
    void handleResponse(int clientFd, Response &res);
};

#endif