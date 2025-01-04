#include "cpp_webserver_include/core.hpp"

#ifdef __linux__
// linux libraries
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
// windows libraries
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <algorithm>
#include <chrono> // For timing
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

HttpServer::HttpServer(const char *ip_address, const char *port)
    : ip_address(ip_address), port(port), server_socket(0) {
}

void HttpServer::start() {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa;
    int init_winsock = WSAStartup(MAKEWORD(2, 0), &wsa);
    if (init_winsock != 0) {
        printf("WSAStartup failed: %d\n", init_winsock);
    }
#endif

    if (createSocket() && bindSocket() && listenSocket()) {
        std::cout << "Server listening on port " << port << std::endl;
        acceptConnections();
    }

// close the server socket
#ifdef __linux__
    close(server_socket);
#elif _WIN32
    closesocket(server_socket);
    // cleanup Winsock
    WSACleanup();
#endif
}

// helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
char *HttpServer::get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
    switch (sa->sa_family) {
        // IPv4
    case AF_INET:
        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
        break;

        // IPv6
    case AF_INET6:
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
        break;

        // Error
    default:
        strncpy(s, "Unknown AF", maxlen);
        return NULL;
    }

    return s;
}

void HttpServer::printRoutes() {
    router.printRoutes();
}

bool HttpServer::createSocket() {
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return false;
    }
    return true;
}

bool HttpServer::bindSocket() {
    // the sockaddr_in structure specifies the address family
    struct sockaddr_in server_address;

    // resets server address
    memset(&server_address, 0, sizeof(server_address));

    // sets IP address, and port to be connected to
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address);
    server_address.sin_port = htons(std::stoi(port));

// set SO_REUSEADDR option to reuse address and prevent being able to start up
#ifdef __linux__
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Setsockopt failed");
        return false;
    }
#elif _WIN32
    const char opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Setsockopt failed");
        return false;
    }
#endif

    // binding socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        return false;
    }

    return true;
}

bool HttpServer::listenSocket() {
    if (listen(server_socket, 10) == -1) {
        perror("Listen failed");
        return false;
    }
    return true;
}

void closeSocket(int client_socket) {
#ifdef __linux__
    close(client_socket);
#elif _WIN32
    closesocket(client_socket);
#endif
}

std::string readSocket(int client_socket) {
    std::string data;
    char buffer[4096];

#ifdef __linux__
    ssize_t bytes_read;
#elif _WIN32
    SSIZE_T bytes_read;
#endif

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read < 0) {
            throw std::runtime_error("Error reading from socket");
        } else if (bytes_read == 0) {
            // End of stream
            break;
        }

        data.append(buffer, bytes_read);

        // Check for end of HTTP request (empty line after headers)
        if (data.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    return data;
}

void sendSocket(int client_socket, const std::string &data) {
    size_t totalBytesSent = 0;
    size_t dataSize = data.size();

    while (totalBytesSent < dataSize) {

#ifdef __linux__
        ssize_t bytesSent;
#elif _WIN32
        SSIZE_T bytesSent;
#endif
        bytesSent =
            send(client_socket, data.c_str() + totalBytesSent, dataSize - totalBytesSent, 0);
        if (bytesSent < 0) {
            throw std::runtime_error("Error sending data to socket");
        }
        totalBytesSent += bytesSent;
    }

    // close the server socket
    closeSocket(client_socket);
}

// Add new middleware method that takes a map of headers
void HttpServer::middleware(const std::map<std::string, std::string> &headers) {
    router.use([headers](Request &req, Response &res) {
        for (const auto &header : headers) {
            res.setSingleHeader(header.first, header.second);
        }
    });
}

void HttpServer::handleRequest(int client_socket) {
    auto start_time = std::chrono::high_resolution_clock::now(); // Start timer

    // reset request / response data
    httpRequest.reset();
    httpResponse.reset();

    std::string requestMessage = readSocket(client_socket);

    // store the request message in the HttpMessage struct
    httpRequest.buildRequest(requestMessage);

    // print request
    std::cout << "HTTP REQUEST MESSAGE: \n"
              << httpRequest.message << std::endl; // Log the raw request message

    // passing request method to response for validation
    httpResponse.setRequestMethod(httpRequest.method);

    if (!router.handleRoute(httpRequest, httpResponse)) {
        logger::error("Route not found: " + httpRequest.uri);
        httpResponse.setHeaders({{"Content-Type", "text/plain"}, {"Connection", "close"}});
        httpResponse.status(404).send("Route not found!");
    }

    handleResponse(client_socket);

    // stop timer and calculate duration
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    // log the duration
    logger::log("Request handled in " + std::to_string(elapsed.count()) + " seconds");
}

void HttpServer::handleResponse(int client_socket) {
    // print response headers
    logger::log("Response Headers:\n");
    for (const auto &header : httpResponse.getHeaders()) {
        std::cout << header.first + ": " + header.second << "\n";
    }
    std::cout << std::endl;

    sendSocket(client_socket, httpResponse.getBody().c_str());
}

void HttpServer::acceptConnections() {
    struct sockaddr_storage client_address;

#ifdef __linux__
    socklen_t client_address_len = sizeof(client_address);
#elif _WIN32
    int client_address_len = sizeof(client_address);
#endif

    while (true) {
        int client_socket =
            accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        logger::section("NEW REQUEST");
        logger::log("Received request from: " + std::to_string(client_socket));
        if (client_socket == -1) {
            perror("Accept failed");
            break;
        }

        // start a new thread for each connection
        threaded_coroutines::Coroutine coroutine([&]() {
            handleRequest(client_socket);
            coroutine.wait();
            threaded_coroutines::yield(); // Yield after each iteration
        });
    }
}

void HttpServer::get(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.get(route, handler);
}
void HttpServer::post(const std::string &route,
                      std::function<void(Request &, Response &)> handler) {
    router.post(route, handler);
}
void HttpServer::put(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.put(route, handler);
}

void HttpServer::del(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.del(route, handler);
}

void HttpServer::use(std::function<void(Request &, Response &)> handler) {
    router.use(handler);
}

void HttpServer::use(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.use(route, handler);
}

HttpServer::~HttpServer() {
// close the server socket
#ifdef __linux__
    close(server_socket);
#elif _WIN32
    closesocket(server_socket);
    // cleanup Winsock
    WSACleanup();
#endif
}