#include "src/hpp_files/request.hpp"
#include "src/hpp_files/response.hpp"
#include "src/hpp_files/http_server.hpp"
#include "src/hpp_files/server_logging.hpp"

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

#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <sstream>
#include <algorithm>

HttpServer::HttpServer(const char *ip_address, const char *port) : ip_address(ip_address), port(port), server_socket(0)
{
}

void HttpServer::start()
{
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa;
    int init_winsock = WSAStartup(MAKEWORD(2, 0), &wsa);
    if (init_winsock != 0)
    {
        printf("WSAStartup failed: %d\n", init_winsock);
    }
#endif

    if (createSocket() && bindSocket() && listenSocket())
    {
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

void HttpServer::addRoute(const std::string &path, std::function<void(Request &, Response &)> handler)
{
    // convert route to lowercase
    std::string lowerCasePath = path;
    transform(lowerCasePath.begin(), lowerCasePath.end(), lowerCasePath.begin(),
              [](unsigned char c)
              { return std::tolower(c); });

    routes[lowerCasePath] = handler;
}

// helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
char *HttpServer::get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch (sa->sa_family)
    {
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

void HttpServer::printRoutes()
{
    logger::log("Possible Routes:");
    for (const auto &pair : routes)
    {
        std::cout << pair.first << "\n";
    }
    std::cout << std::endl;
}

bool HttpServer::createSocket()
{
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        return false;
    }
    return true;
}

bool HttpServer::bindSocket()
{
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
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("Setsockopt failed");
        return false;
    }
#elif _WIN32
    const char opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("Setsockopt failed");
        return false;
    }
#endif

    // binding socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Bind failed");
        return false;
    }

    return true;
}

bool HttpServer::listenSocket()
{
    if (listen(server_socket, 10) == -1)
    {
        perror("Listen failed");
        return false;
    }
    return true;
}

std::string extractMainRoute(const std::string &url)
{
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos)
    {
        return url.substr(0, queryPos);
    }
    else
    {
        return url;
    }
}

void HttpServer::handleRequest(int client_socket)
{
    std::cout.flush();
    char buffer[3060];
    memset(buffer, 0, 3060); // resetting buffer between requests

    int byte_count_transfer = 0;
#ifdef __linux__
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    do
    {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_read);
#elif _WIN32
    SSIZE_T bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    do
    {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_read);
#endif

    std::string request(buffer);

    // print request
    logger::section("NEW REQUEST");
    logger::log("Received Request:\n");
    std::cout << request;

    // parse the request to determine the type (e.g., GET, POST) and extract relevant information
    parseHttpRequest(request);
    httpRequest.setParams("");                              // resetting params after each request
    httpRequest.setParams(httpRequest.getPath());           // parse url params and set them for the request
    httpResponse.setRequestMethod(httpRequest.getMethod()); // passing request method to response for validation

    // print params
    std::cout << "Params:\n";
    for (const auto &params : httpRequest.getParams())
    {
        std::cout << params.first << " : " << params.second << std::endl;
    };

    // setting main route for lookup
    httpRequest.setPath(extractMainRoute(httpRequest.getPath()));

    if (checkRoutes())
    {
        // if route exists
        routes.find(httpRequest.getPath())->second(httpRequest, httpResponse);
        handleResponse(client_socket);

// close the server socket
#ifdef __linux__
        close(client_socket);
#elif _WIN32
        closesocket(client_socket);
#endif
    }
    else
    {
        // if route doesn't exist
        logger::error("client_socket " + std::to_string(client_socket) + " -- route does not exist...");

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

void HttpServer::handleResponse(int client_socket)
{
    // print response headers
    logger::log("Response Headers:\n");
    for (const auto &header : httpResponse.getHeaders())
    {
        std::cout << header.first + ": " + header.second << "\n";
    }
    std::cout << std::endl;

    // while bytes_written is less than byte_count_transfer
    int byte_count_transfer = 0;
    // logger::log("Response Body: " + httpResponse.getBody());

#ifdef __linux__
    // send response to client socket
    ssize_t bytes_sent = send(client_socket, httpResponse.getBody().c_str(), strlen(httpResponse.getBody().c_str()), 0);
    do
    {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_sent);
    logger::log("Sent response to client socket " + std::to_string(client_socket));

#elif _WIN32
    // send response to client socket
    SSIZE_T bytes_sent = send(client_socket, httpResponse.getBody().c_str(), strlen(httpResponse.getBody().c_str()), 0);
    do
    {
        byte_count_transfer++;
    } while (byte_count_transfer <= bytes_sent);
    logger::log("Sent response to client socket " + std::to_string(client_socket));

#endif
}

void HttpServer::acceptConnections()
{
    struct sockaddr_storage client_address;

#ifdef __linux__
    socklen_t client_address_len = sizeof(client_address);
#elif _WIN32
    int client_address_len = sizeof(client_address);
#endif

    while (true)
    {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        logger::log("Received request from: " + std::to_string(client_socket));
        if (client_socket == -1)
        {
            perror("Accept failed");
            break;
        }

        // start a new thread for each connection
        std::thread client_thread(&HttpServer::handleRequest, this, client_socket);
        client_thread.detach(); // detach the thread to allow it to run independently
    }
}

void HttpServer::parseHttpRequest(const std::string &requestBuffer)
{
    // find the position of the first '\r\n'
    size_t endOfFirstLine = requestBuffer.find("\r\n");

    if (endOfFirstLine != std::string::npos)
    {
        // extract the first line
        std::string firstLine = requestBuffer.substr(0, endOfFirstLine);

        // parse the first line (assuming "METHOD /route HTTP/1.1")
        std::istringstream iss(firstLine);
        std::string method, route, httpVersion;

        if (iss >> method >> route >> httpVersion)
        {
            // Store the parsed information in the map
            httpRequest.setMethod(method);
            httpRequest.setPath(route);
            // httpRequest.setVersion(httpVersion);
        }
        else
        {
            logger::error("Failed to parse the first line of the HTTP request.");
        }
    }
    else
    {
        logger::error("No valid HTTP request found in the buffer.");
    }
}

bool HttpServer::checkRoutes()
{
    try
    {
        std::string requestRoute = httpRequest.getPath();
        logger::log("Received route \"" + requestRoute + "\"");
        for (const auto &route : routes)
        {
            // std::cout << "Route first: " << route.first << std::endl;
            if (route.first == requestRoute)
            {
                return true;
            }
        }
    }
    catch (MyCustomException error)
    {
        logger::error(error.what());
    }
    return false;
}

HttpServer::~HttpServer()
{
// close the server socket
#ifdef __linux__
    close(server_socket);
#elif _WIN32
    closesocket(server_socket);
    // cleanup Winsock
    WSACleanup();
#endif
}