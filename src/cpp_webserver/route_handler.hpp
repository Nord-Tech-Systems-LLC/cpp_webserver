#pragma once

#ifndef ROUTE_HANDLER_H
#define ROUTE_HANDLER_H

#include <functional>
#include <map>
#include <string>

class RouteHandler {
   public:
    void addRoute(const std::string& path, std::function<void(int)> handler);
    bool verifyRouteExists(int client_socket, const std::string& request);
    bool checkRoutes(const std::string& route_request);

   private:
    std::map<std::string, std::string> parseHttpRequest(const std::string& requestBuffer);
    std::map<std::string, std::function<void(int)>> routes;
    // map to store method, route, and http version
    std::map<std::string, std::string> parsedInfo;
};

#endif