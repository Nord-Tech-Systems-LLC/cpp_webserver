#include "cpp_webserver/route_handler.hpp"

#include <unistd.h>

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cpp_webserver/server_logging.hpp"

void RouteHandler::addRoute(const std::string& path, std::function<void(int)> handler) {
    routes[path] = handler;
}

bool RouteHandler::checkRoutes(const std::string& route_request) {
    try {
        // parsing and getting first line of request
        std::map<std::string, std::string> requestDataMap = parseHttpRequest(route_request);

        std::string requestRoute = requestDataMap["route"];
        std::cout << "[request route]: " << requestRoute << std::endl;

        for (const auto& route : routes) {
            // std::cout << "Route first: " << route.first << std::endl;
            if (route.first == requestRoute) {
                std::cout << "[correct route]: " << route.first << std::endl;
                return true;
            }
        }

    } catch (MyCustomException error) {
        logger::exitWithError(error.what());
    }
    return false;
}

std::map<std::string, std::string> RouteHandler::parseHttpRequest(const std::string& requestBuffer) {
    // map to store method, route, and http version
    std::map<std::string, std::string> parsedInfo;

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

std::string returnRoute(const std::string& route_input) {
    size_t firstSpace = route_input.find(' ');
    if (firstSpace == std::string::npos) {
        return "";  // Return an empty string if space is not found
    }
    size_t secondSpace = route_input.find(' ', firstSpace + 1);
    if (secondSpace == std::string::npos) {
        return "";  // Return an empty string if second space is not found
    }

    return route_input.substr(firstSpace + 1, secondSpace - firstSpace - 1);
}

bool RouteHandler::handleRequest(int client_socket, const std::string& request) {
    std::cout << "Request: " << request << std::endl;
    bool route_exists = false;
    for (const auto& route : routes) {
        // check if route exists
        if (returnRoute(request) == route.first) {
            route.second(client_socket);
            close(client_socket);
            route_exists = true;
        }
    }
    return route_exists;  // No matching route found
}
