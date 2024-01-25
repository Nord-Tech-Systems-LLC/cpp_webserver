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
        std::string requestRoute = parsedInfo["route"];

        for (const auto& route : routes) {
            // std::cout << "Route first: " << route.first << std::endl;
            if (route.first == requestRoute) {
                logger::log("Correct route \"" + route.first + "\" found.");
                return true;
            }
        }

    } catch (MyCustomException error) {
        logger::exitWithError(error.what());
    }
    return false;
}

// bool RouteHandler::verifyRouteExists(int client_socket, const std::string& request) {
//     std::cout << "Request: " << request << std::endl;
//     // parseHttpRequest(request);

//     bool route_exists = false;
//     for (const auto& route : routes) {
//         // check if route exists
//         if (parsedInfo["route"] == route.first) {
//             route.second(client_socket);
//             // close(client_socket);
//             route_exists = true;
//         }
//     }
//     return route_exists;  // No matching route found
// }

// void RouteHandler::