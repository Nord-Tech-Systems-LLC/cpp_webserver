#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <thread>

#include "cpp_webserver/server_logging.hpp"
// #include "route_handler.cpp"

#include <functional>
#include <map>
#include <string>

class RouteHandler {
   public:
    void addRoute(const std::string &path, std::function<void(int)> handler) {
        routes[path] = handler;
    }

    bool handleRequest(int client_socket, const std::string &request) {
        for (const auto &route : routes) {
            if (request.find(route.first) != std::string::npos) {
                route.second(client_socket);
                return true;
            }
        }
        return false;  // No matching route found
    }

   private:
    std::map<std::string, std::function<void(int)>> routes;
};

class HttpServer {
   public:
    HttpServer(const char *port) : port(port), server_socket(0) {
        // routeHandler.addRoute("/", std::bind(&HttpServer::sendHttpGetResponse, this, std::placeholders::_1));
        // routeHandler.addRoute("/hello", std::bind(&HttpServer::sendHttpGetResponse, this, std::placeholders::_1));
    }

    void start() {
        if (createSocket() && bindSocket() && listenSocket()) {
            std::cout << "Server listening on port " << port << std::endl;
            acceptConnections();
        }
        close(server_socket);
    }

   private:
    const char *port;
    int server_socket;
    // RouteHandler routeHandler;

    // Helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
        switch (sa->sa_family) {
            case AF_INET:
                inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
                break;

            case AF_INET6:
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
                break;

            default:
                strncpy(s, "Unknown AF", maxlen);
                return NULL;
        }

        return s;
    }

    bool createSocket() {
        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket creation failed");
            return false;
        }
        return true;
    }

    bool bindSocket() {
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

    bool listenSocket() {
        if (listen(server_socket, 10) == -1) {
            perror("Listen failed");
            return false;
        }
        return true;
    }

    void handleRequest(int client_socket) {
        char buffer[1024] = {0};
        read(client_socket, buffer, sizeof(buffer));

        // Simple response
        // const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
        // write(client_socket, response, strlen(response));

        // Parse the request to determine the type (e.g., GET, POST) and extract relevant information
        // For simplicity, let's assume a basic HTTP GET request
        std::string request(buffer);

        // std::cout << request << std::endl;
        // Check if it's a GET request
        if (request.find("GET /test HTTP/1.1") != std::string::npos) {
            logger::log("\n" + request);
            sendHttpGetResponse(client_socket);
        } else {
            sendCustomResponse(client_socket, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        }
        logger::log("Sent response...");

        close(client_socket);
    }

    void acceptConnections() {
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

    void sendHttpGetResponse(int client_socket) {
        // Custom response for a GET request
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
        write(client_socket, response, strlen(response));
    }

    void sendCustomResponse(int client_socket, const char *response) {
        // Send a custom response
        write(client_socket, response, strlen(response));
    }
};

int main() {
    HttpServer server("8080");
    server.start();

    return 0;
}
