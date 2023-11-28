#include "cpp_webserver/route_handler.hpp"

#include <functional>
#include <map>
#include <string>

class RouteHandler {
   public:
    void addRoute(const std::string& path, std::function<void(int)> handler) {
        routes[path] = handler;
    }

    bool handleRequest(int client_socket, const std::string& request) {
        for (const auto& route : routes) {
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