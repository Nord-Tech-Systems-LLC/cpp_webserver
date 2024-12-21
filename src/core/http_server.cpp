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

void HttpServer::handleRequest(int client_socket) {
    auto start_time = std::chrono::high_resolution_clock::now(); // Start timer

    // reset request / response data
    httpRequest.reset();
    httpResponse.reset();

    std::string requestMessage = readSocket(client_socket);
    httpRequest.setMessage(requestMessage); // Store the request message in the HttpMessage struct

    // print request
    std::cout << "HTTP REQUEST MESSAGE: \n"
              << httpRequest.getMessage() << std::endl; // Log the raw request message

    // extract the method, URI, and HTTP version (can be extracted from the message directly)
    size_t method_end = httpRequest.getMessage().find(" ");
    size_t uri_end = httpRequest.getMessage().find(" ", method_end + 1);
    size_t proto_end = httpRequest.getMessage().find("\r\n", uri_end + 1);

    // ensure each part is found
    if (method_end == std::string::npos || uri_end == std::string::npos ||
        proto_end == std::string::npos) {
        std::string response =
            httpResponse.buildResponse("400 Bad Request",
                                       {{"Content-Type", "text/plain"}, {"Connection", "close"}},
                                       "Malformed request line");

        httpResponse.setBody(response);
        handleResponse(client_socket);
        return; // stop processing this request
    }

    // extract method, URI, and HTTP version
    std::string method = httpRequest.getMessage().substr(0, method_end);
    std::string uri = httpRequest.getMessage().substr(method_end + 1, uri_end - method_end - 1);
    std::string proto = httpRequest.getMessage().substr(uri_end + 1, proto_end - uri_end - 1);

    // set the values in the HttpRequest object
    httpRequest.setMethod(method);
    httpRequest.setUri(uri);
    httpRequest.setProto(proto);

    // set headers by parsing request
    std::vector<HttpHeader> newHeaders = {};
    extractHttpHeader(newHeaders, httpRequest.getMessage());
    httpRequest.setHeaders(newHeaders);          // setting headers after each request
    httpRequest.setParams("");                   // resetting params after each request
    httpRequest.setParams(httpRequest.getUri()); // parse url params and set them for the request
    httpResponse.setRequestMethod(
        httpRequest.getMethod()); // passing request method to response for validation

    // setting main route for lookup
    httpRequest.setUri(extractMainRoute(httpRequest.getUri()));

    if (check_routes()) {
        // if route exists
        routes.find(route_template)->second(httpRequest, httpResponse);
        handleResponse(client_socket);

    } else {
        // if route doesn't exist
        logger::error("client_socket " + std::to_string(client_socket) +
                      " -- route does not exist...");

        // set headers
        httpResponse.setHeaders({{"Content-Type", "text/html"}, {"Connection", "close"}});
        // error response
        std::string errorResponse = "Route does not exist!";
        std::string response =
            httpResponse.buildResponse("400 Bad Request", httpResponse.getHeaders(), errorResponse);
        httpResponse.setBody(response);
        handleResponse(client_socket);
    }

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

        // std::thread client_thread(&HttpServer::handleRequest, this, client_socket);
        // client_thread.detach(); // detach the thread to allow it to run independently
    }
}

bool HttpServer::is_route_match(const std::string &routePattern, const std::string &requestUri) {
    // split the route pattern and request URI into segments
    auto routeSegments = httpRequest.split_path(routePattern);
    auto requestSegments = httpRequest.split_path(requestUri);

    // if the number of segments doesn't match, the route doesn't match
    if (routeSegments.size() != requestSegments.size()) {
        return false;
    }

    // check each segment for a match
    for (size_t i = 0; i < routeSegments.size(); ++i) {
        if (routeSegments[i].front() == ':') {
            // if the route segment starts with ':', treat it as a wildcard
            continue;
        } else if (routeSegments[i] != requestSegments[i]) {
            // if the segments don't match, return false
            return false;
        }
    }

    // all segments match
    return true;
}

bool HttpServer::check_routes() {
    try {
        std::string request_route = httpRequest.getUri();

        // catch all for routes not to be too long
        if (request_route.length() > 2048) {
            logger::error("Route is too long, please try again.");
            return false;
        };

        logger::log("Received route \"" + request_route + "\"");

        // find route template created at beginning routes
        for (const auto &route : routes) {
            if (is_route_match(route.first, request_route)) {
                route_template = route.first;
                httpRequest.setPathParams(route_template, request_route);
            }
        }

        // check method matches expected
        for (const auto &route : method_route_pair) {
            if (route.first == route_template) {
                if (route.second != httpRequest.getMethod()) {
                    logger::error("Incorrect API request method: " + httpRequest.getMethod() +
                                  " Expected: " + route.second);
                    return false;
                }
            }
        }

        // split paths into vector for easier access and indexing
        auto request_segments = httpRequest.split_path(request_route);
        auto route_segments = httpRequest.split_path(route_template);

        // find route template created at beginning routes
        for (const auto &route : routes) {
            if (is_route_match(route.first, request_route)) {

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