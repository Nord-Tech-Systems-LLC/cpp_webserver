

#include "cpp_webserver/http_server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <sstream>

#include "cpp_webserver/server_logging.hpp"

HttpServer::HttpServer(const char *port) : port(port), server_socket(0) {
    // routeHandler.addRoute("/", std::bind(&HttpServer::sendHttpGetResponse, this, std::placeholders::_1));
    // routeHandler.addRoute("/hello", std::bind(&HttpServer::sendHttpGetResponse, this, std::placeholders::_1));
}

void HttpServer::start() {
    if (createSocket() && bindSocket() && listenSocket()) {
        std::cout << "Server listening on port " << port << std::endl;
        acceptConnections();
    }
    close(server_socket);
}

void HttpServer::addRoute(const std::string &path, std::function<void(Request&, Response&)> handler) {
    routes[path] = handler;
}

// Helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
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

void HttpServer::print_routes() {
       for (const auto& pair : routes) {
        std::cout << pair.first << std::endl;
    }
}

bool HttpServer::createSocket() {
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return false;
    }
    return true;
}

bool HttpServer::bindSocket() {
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(std::stoi(port));

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

void HttpServer::handleRequest(int client_socket) {
    std::cout.flush();
    // std::cin.clear();
    char buffer[3060];
    read(client_socket, buffer, sizeof(buffer));
    // logger::log(buffer);

    // parse the request to determine the type (e.g., GET, POST) and extract relevant information
    std::string request(buffer);

    logger::log("Sent response...");
    parseHttpRequest(request);

    // if route exists
    if (checkRoutes(request)) {
        for (const auto &route : routes) {

            // check if route exists
            if (parsedInfo["route"] == route.first) {
                route.second(httpRequest, httpResponse);
                // while bytes_written is less than byte_count_transfer
                int byte_count_transfer = 0;
                logger::log("Response Body: " + httpResponse.getBody());
                ssize_t bytes_written = write(client_socket, httpResponse.getBody().c_str(), strlen(httpResponse.getBody().c_str()));
                do {
                    byte_count_transfer++;
                } while (byte_count_transfer <= bytes_written);
            }

        }
        close(client_socket);
    } else {
        logger::exitWithError("client_socket " + std::to_string(client_socket) + " -- route does not exist...");
        sendCustomResponse(client_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 90\r\n\r\nThere was an error!");
        close(client_socket);
    }
}

void HttpServer::acceptConnections() {
    struct sockaddr_storage client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        logger::log("Received request from: " + std::to_string(client_socket));
        if (client_socket == -1) {
            perror("Accept failed");
            break;
        }

        handleRequest(client_socket);
        // Start a new thread for each connection
        // std::thread client_thread(&HttpServer::handleRequest, this, client_socket);

        // client_thread.detach();  // Detach the thread to allow it to run independently
    }
}

std::map<std::string, std::string> HttpServer::parseHttpRequest(const std::string &requestBuffer) {
    // Find the position of the first '\r\n'
    size_t endOfFirstLine = requestBuffer.find("\r\n");

    if (endOfFirstLine != std::string::npos) {
        // Extract the first line
        std::string firstLine = requestBuffer.substr(0, endOfFirstLine);

        // Parse the first line (assuming "METHOD /route HTTP/1.1")
        std::istringstream iss(firstLine);
        std::string method, route, httpVersion;

        if (iss >> method >> route >> httpVersion) {
            // Store the parsed information in the map
            parsedInfo["method"] = method;
            parsedInfo["route"] = route;
            parsedInfo["http_version"] = httpVersion;

        } else {
            std::cerr << "Failed to parse the first line of the HTTP request." << std::endl;
        }
    } else {
        std::cerr << "No valid HTTP request found in the buffer." << std::endl;
    }
    return parsedInfo;
}

void HttpServer::sendHttpGetResponse(int client_socket) {
    // Custom response for a GET request
    // const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
    // write(client_socket, response, strlen(response));
}

void HttpServer::sendCustomResponse(int client_socket, const char *response) {
    // Send a custom response
    write(client_socket, response, strlen(response));
    close(client_socket);
}

bool HttpServer::checkRoutes(const std::string& route_request) {
    try {
        std::string requestRoute = parsedInfo["route"];
        logger::log("Received route \"" + requestRoute + "\"");
        for (const auto& route : routes) {
            // std::cout << "Route first: " << route.first << std::endl;
            if (route.first == requestRoute) {
                return true;
            }
        }

    } catch (MyCustomException error) {
        logger::exitWithError(error.what());
    }
    return false;
}