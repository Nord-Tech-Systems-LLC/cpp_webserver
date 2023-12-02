

#include "cpp_webserver/http_server.hpp"
#include "http_request.cpp"
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

void HttpServer::addRoute(const std::string &path, std::function<void(int)> handler) {
    routeHandler.addRoute(path, handler);
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
    // std::cout.flush();
    // std::cin.clear();
    char buffer[3060];
    read(client_socket, buffer, sizeof(buffer));
    // logger::log(buffer);

    // parse the request to determine the type (e.g., GET, POST) and extract relevant information
    std::string request(buffer);
    // HttpRequest parsedRequest = parseHttpRequest(request);

    // to parse and get headers
    // for (const auto &header: parsedRequest.headers) {
    //     // to parse and get headers
    //     std::cout << header.first << ": "
    //     << header.second << "\n";
    // }

    // // for debugging
    // std::cout << "Method: " << parsedRequest.method << std::endl;
    // std::cout << "Path: " << parsedRequest.path << std::endl;
    // std::cout << "Body: " << parsedRequest.body << std::endl;

    // logger::log("Sending response...");
    // bool route_exists = routeHandler.handleRequest(client_socket, request);
    // std::cout << request << std::endl;
    // HandleHttpRequest readRequest;

    // const char *otherHtmlResponse = "HTTP/1.1 200 OK\r\nContent-Length: 90\r\n\r\n<html><body>This was other!</body></html>";
    // readRequest.handleConnection(client_socket, otherHtmlResponse);
    logger::log("Sent response...");
    // if route does not exist
    if (!routeHandler.handleRequest(client_socket, request)) {
        logger::log("client_socket" + client_socket);
        sendCustomResponse(client_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 90\r\n\r\nThere was an error!");
    } else {
        // if (routeHandler.checkRoutes(request)) {
        //     // sendHttpGetResponse(client_socket);
        // } else {
        //     sendCustomResponse(client_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 90\r\n\r\nThere was an error!");
        // }
        routeHandler.handleRequest(client_socket, request);
        
        close(client_socket);

    }




}

void HttpServer::acceptConnections() {
    struct sockaddr_storage client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        logger::log("Received request...");
        if (client_socket == -1) {
            perror("Accept failed");
            break;
        }

        // Read the request from the client
        // char buffer[1024] = {0};
        // read(client_socket, buffer, sizeof(buffer));

        // Handle the request using the RouteHandler
        // if (!routeHandler.handleRequest(client_socket, buffer)) {
        //     // No matching route found, send a 404 response
        //     sendCustomResponse(client_socket, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        //     close(client_socket);
        // }

        // Start a new thread for each connection
        // std::thread client_thread(&HttpServer::handleRequest, this, client_socket);
        // client_thread.detach();  // Detach the thread to allow it to run independently

        handleRequest(client_socket);
    }
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
