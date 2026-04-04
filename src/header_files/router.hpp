#ifndef ROUTER_H
#define ROUTER_H

#include "cpp_webserver_include/core.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>

class Request;  // forward declare
class Response; // forward declare

class Router {
  public:
    Router() = default;
    ~Router() = default;

    // Route registration methods
    void get(const std::string &route, std::function<void(Request &, Response &)> handler);
    void post(const std::string &route, std::function<void(Request &, Response &)> handler);
    void put(const std::string &route, std::function<void(Request &, Response &)> handler);
    void del(const std::string &route, std::function<void(Request &, Response &)> handler);

    // Middleware support
    using MiddlewareFunction = std::function<void(Request &, Response &)>;
    void use(MiddlewareFunction middleware);                          // Global middleware
    void use(const std::string &path, MiddlewareFunction middleware); // Path-specific middleware

    // Dispatch
    bool handleRoute(Request &req, Response &res);

    // Introspection
    void printRoutes() const;
    std::string findMatchingRouteTemplate(const std::string &requestUri) const;

    // RFC 2616 §9.2 / §10.4.6 — return every HTTP method registered for a URI.
    // Used to populate the Allow header in 405 responses and OPTIONS replies.
    std::vector<std::string> allowedMethods(const std::string &requestUri) const;

  private:
    void addRoute(const std::string &method, const std::string &route,
                  std::function<void(Request &, Response &)> handler);
    bool isRouteMatch(const std::string &routePattern, const std::string &requestUri) const;
    bool isPathMatch(const std::string &middlewarePath, const std::string &requestUri) const;

    struct RouteInfo {
        std::string method;
        std::function<void(Request &, Response &)> handler;
    };

    struct MiddlewareInfo {
        std::string path;
        MiddlewareFunction handler;
        bool isGlobal;
    };

    std::map<std::string, RouteInfo> routes;
    std::vector<MiddlewareInfo> middlewareStack;
};

#endif