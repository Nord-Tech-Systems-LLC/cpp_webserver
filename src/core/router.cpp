#include "cpp_webserver_include/core.hpp"
#include <algorithm>
#include <iostream>

void Router::addRoute(const std::string &method,
                      const std::string &route,
                      std::function<void(Request &, Response &)> handler) {
    std::string lowerCaseRoute = route;
    std::transform(lowerCaseRoute.begin(), lowerCaseRoute.end(), lowerCaseRoute.begin(), ::tolower);

    routes[lowerCaseRoute] = RouteInfo{method, handler};
}

void Router::get(const std::string &route, std::function<void(Request &, Response &)> handler) {
    addRoute("GET", route, handler);
}

void Router::post(const std::string &route, std::function<void(Request &, Response &)> handler) {
    addRoute("POST", route, handler);
}

void Router::put(const std::string &route, std::function<void(Request &, Response &)> handler) {
    addRoute("PUT", route, handler);
}

void Router::del(const std::string &route, std::function<void(Request &, Response &)> handler) {
    addRoute("DELETE", route, handler);
}

void Router::use(MiddlewareFunction middleware) {
    // Global middleware
    middlewareStack.push_back({"", // Empty path means global
                               middleware,
                               true});
}

void Router::use(const std::string &path, MiddlewareFunction middleware) {
    // Path-specific middleware
    middlewareStack.push_back({path, middleware, false});
}

bool Router::isRouteMatch(const std::string &routePattern, const std::string &requestUri) const {

    auto splitPath = [](const std::string &path) {
        std::vector<std::string> segments;
        std::string segment;
        std::istringstream stream(path);

        while (std::getline(stream, segment, '/')) {
            if (!segment.empty()) {
                segments.push_back(segment);
            }
        }
        return segments;
    };

    auto routeSegments = splitPath(routePattern);
    auto requestSegments = splitPath(requestUri);

    if (routeSegments.size() != requestSegments.size()) {
        return false;
    }

    for (size_t i = 0; i < routeSegments.size(); ++i) {
        if (routeSegments[i].front() == ':') {
            continue;
        }
        if (routeSegments[i] != requestSegments[i]) {
            return false;
        }
    }

    return true;
}

bool Router::isPathMatch(const std::string &middlewarePath, const std::string &requestUri) const {

    // If middleware path is empty (global middleware) or paths are equal, it's a match
    if (middlewarePath.empty() || middlewarePath == requestUri) {
        return true;
    }

    // Check if the request URI starts with the middleware path
    // This allows middleware to match all routes under a specific path
    if (requestUri.find(middlewarePath) == 0) {
        // Make sure we match complete path segments
        // e.g., "/api" should match "/api/users" but not "/api-users"
        if (requestUri.length() == middlewarePath.length() ||
            requestUri[middlewarePath.length()] == '/') {
            return true;
        }
    }

    return false;
}

std::string Router::findMatchingRouteTemplate(const std::string &requestUri) const {
    for (const auto &route : routes) {
        if (isRouteMatch(route.first, requestUri)) {
            // routeMatch = route.first;
            return route.first;
        }
    }
    return "";
}

bool Router::handleRoute(Request &req, Response &res) {
    try {
        // Apply middleware
        for (const auto &middleware : middlewareStack) {
            // Check if middleware should be applied to this route
            if (middleware.isGlobal || isPathMatch(middleware.path, req.uri)) {
                middleware.handler(req, res);
            }
        }

        std::string routeTemplate = findMatchingRouteTemplate(req.uri);
        if (routeTemplate.empty()) {
            return false;
        }

        const auto &routeInfo = routes.at(routeTemplate);
        if (routeInfo.method != req.method) {
            return false;
        }

        // Execute the route handler
        routeInfo.handler(req, res);
        return true;

    } catch (const std::exception &e) {
        // Handle exception thrown by middleware or route handler
        std::cout << e.what() << std::endl;
        return true; // Stop further processing
    }
}

void Router::printRoutes() const {
    std::cout << "Registered Routes:" << std::endl;
    for (const auto &route : routes) {
        std::cout << route.second.method << " " << route.first << std::endl;
    }
}