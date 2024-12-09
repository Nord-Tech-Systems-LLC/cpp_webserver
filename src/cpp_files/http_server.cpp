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

void HttpServer::addRoute(const std::string &method,
                          const std::string &route,
                          std::function<void(Request &, Response &)> handler) {
    // convert route to lowercase
    std::string lowerCaseRoute = route;

    transform(
        lowerCaseRoute.begin(), lowerCaseRoute.end(), lowerCaseRoute.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

    int route_index = 0;
    method_route_pair[lowerCaseRoute] = method;
    routes[lowerCaseRoute] = handler;
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
    logger::log("Possible Routes:");
    for (const auto &pair : routes) {
        std::cout << pair.first << "\n";
    }
    std::cout << std::endl;
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

std::string extractMainRoute(const std::string &url) {
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        return url.substr(0, queryPos);
    } else {
        return url;
    }
}

void HttpServer::handleRequest(int client_socket) {
    std::cout.flush();
    char buffer[3060];
    memset(buffer, 0, 3060); // resetting buffer between requests

    int byte_count_transfer = 0;
#ifdef __linux__
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    do {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_read);
#elif _WIN32
    SSIZE_T bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    do {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_read);
#endif

    httpRequest.setMessage(
        std::string(buffer, bytes_read)); // Store the request message in the HttpMessage struct

    // print request
    logger::section("NEW REQUEST");
    logger::log("Received Request:\n");
    std::cout << httpRequest.getMessage() << std::endl; // Log the raw request message

    // Extract the method, URI, and HTTP version (can be extracted from the message directly)
    size_t method_end = httpRequest.getMessage().find(" ");
    size_t uri_end = httpRequest.getMessage().find(" ", method_end + 1);
    size_t proto_end = httpRequest.getMessage().find("\r\n", uri_end + 1);

    // Ensure each part is found
    if (method_end == std::string::npos || uri_end == std::string::npos ||
        proto_end == std::string::npos) {
        throw std::invalid_argument("Malformed request line");
    }

    httpRequest.setMethod(httpRequest.getMessage().substr(0, method_end));
    httpRequest.setUri(httpRequest.getMessage().substr(method_end + 1, uri_end - method_end - 1));
    httpRequest.setProto(httpRequest.getMessage().substr(uri_end + 1, proto_end - uri_end - 1));

    // Set headers by parsing request
    std::vector<HttpHeader> newHeaders = {};
    extractHttpHeader(newHeaders, httpRequest.getMessage());
    httpRequest.setHeaders(newHeaders);          // Setting headers after each request
    httpRequest.setParams("");                   // resetting params after each request
    httpRequest.setParams(httpRequest.getUri()); // parse url params and set them for the request
    httpResponse.setRequestMethod(
        httpRequest.getMethod()); // passing request method to response for validation

    // setting main route for lookup
    logger::log("BEFORE EXTRACT ROUTE" + httpRequest.getUri());
    httpRequest.setUri(extractMainRoute(httpRequest.getUri()));

    // std::cout << "Headers:\n";
    // for (const auto &header : httpRequest.getHeaders())
    // {
    //     std::cout << header.name << ": " << header.value << std::endl;
    // }

    // print params
    // std::cout << "Params:\n";
    // for (const auto &params : httpRequest.getParams())
    // {
    //     std::cout << params.first << " : " << params.second << std::endl;
    // };

    if (checkRoutes()) {
        // if route exists
        logger::log("TESTING: " + httpRequest.getUri());
        routes.find(httpRequest.getUri())->second(httpRequest, httpResponse);
        handleResponse(client_socket);

// close the server socket
#ifdef __linux__
        close(client_socket);
#elif _WIN32
        closesocket(client_socket);
#endif
    } else {
        // if route doesn't exist
        logger::error("client_socket " + std::to_string(client_socket) +
                      " -- route does not exist...");

        // set headers
        httpResponse.setHeaders({{"Content-Type", "text/html"},
                                 {"Connection", "keep-alive"},
                                 {
                                     "Accept-Encoding",
                                     "gzip, deflate, br",
                                 }});

        // error response
        std::string errorResponse = "Route does not exist!";
        std::string response = httpResponse.buildResponse("400 Bad Request", errorResponse);
        httpResponse.setBody(response);
        handleResponse(client_socket);

// close the server socket
#ifdef __linux__
        close(client_socket);
#elif _WIN32
        closesocket(client_socket);
#endif
    }
}

void HttpServer::handleResponse(int client_socket) {
    // print response headers
    logger::log("Response Headers:\n");
    for (const auto &header : httpResponse.getHeaders()) {
        std::cout << header.first + ": " + header.second << "\n";
    }
    std::cout << std::endl;

    // while bytes_written is less than byte_count_transfer
    int byte_count_transfer = 0;
    // logger::log("Response Body: " + httpResponse.getBody());

#ifdef __linux__
    // send response to client socket
    ssize_t bytes_sent = send(
        client_socket, httpResponse.getBody().c_str(), strlen(httpResponse.getBody().c_str()), 0);
    do {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_sent);
    logger::log("Sent response to client socket " + std::to_string(client_socket));

#elif _WIN32
    // send response to client socket
    SSIZE_T bytes_sent = send(
        client_socket, httpResponse.getBody().c_str(), strlen(httpResponse.getBody().c_str()), 0);
    do {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_sent);
    logger::log("Sent response to client socket " + std::to_string(client_socket));

#endif
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
        logger::log("Received request from: " + std::to_string(client_socket));
        if (client_socket == -1) {
            perror("Accept failed");
            break;
        }

        // start a new thread for each connection
        std::thread client_thread(&HttpServer::handleRequest, this, client_socket);
        client_thread.detach(); // detach the thread to allow it to run independently
    }
}

bool HttpServer::checkRoutes() {
    try {
        std::string requestRoute = httpRequest.getUri();
        logger::log("Received route \"" + requestRoute + "\"");

        // check method matches expected
        for (const auto &route : method_route_pair) {
            if (route.first == requestRoute) {
                if (route.second != httpRequest.getMethod()) {
                    logger::error("Incorrect API request method: " + httpRequest.getMethod() +
                                  " Expected: " + route.second);
                    return false;
                }
            }
        }

        // check route exists
        for (const auto &route : routes) {
            if (route.first == requestRoute) {
                return true;
            }
        }

    } catch (MyCustomException error) {
        logger::error(error.what());
    }
    return false;
}

void HttpServer::extractHttpHeader(std::vector<HttpHeader> &headerVector,
                                   const std::string &message) {
    for (int i = 0; i < MAX_HTTP_HEADERS; ++i) {
        // Get header name (from the message) and its value
        headerVector.clear();
        std::istringstream stream(message);
        std::string line;

        // Skip the request line (first line)
        std::getline(stream, line);

        // Loop through each subsequent line until we find an empty line
        while (std::getline(stream, line) && !line.empty() && line != "\r") {
            // Each header line is in the format: "Header-Name: Header-Value"
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string name = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);

                // Trim any leading whitespace in the value
                size_t firstNonSpace = value.find_first_not_of(" \t");
                if (firstNonSpace != std::string::npos) {
                    value = value.substr(firstNonSpace);
                }
                headerVector.push_back({name, value});
            }
        }
    }
}

void HttpServer::get(const std::string &route, std::function<void(Request &, Response &)> handler) {
    addRoute("GET", route, handler);
}
void HttpServer::post(const std::string &route,
                      std::function<void(Request &, Response &)> handler) {
    addRoute("POST", route, handler);
}
void HttpServer::put(const std::string &route, std::function<void(Request &, Response &)> handler) {
    addRoute("PUT", route, handler);
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