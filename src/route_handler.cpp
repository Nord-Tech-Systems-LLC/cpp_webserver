#include "cpp_webserver/route_handler.hpp"

#include <functional>
#include <map>
#include <string>

void RouteHandler::addRoute(const std::string& path, std::function<void(int)> handler) {
    routes[path] = handler;
}

bool RouteHandler::handleRequest(int client_socket, const std::string& request) {
    for (const auto& route : routes) {
        if (request.find(route.first) != std::string::npos) {
            route.second(client_socket);
            return true;
        }
    }
    return false;  // No matching route found
}
