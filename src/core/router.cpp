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

void Router::use(MiddlewareFunction middleware) {
    middlewareStack.push_back(middleware);
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

std::string Router::findMatchingRouteTemplate(const std::string &requestUri) const {
    for (const auto &route : routes) {
        if (isRouteMatch(route.first, requestUri)) {
            return route.first;
        }
    }
    return "";
}

bool Router::handleRoute(Request &req, Response &res) {
    // Apply middleware first
    for (const auto &middleware : middlewareStack) {
        middleware(req, res);
    }

    std::string routeTemplate = findMatchingRouteTemplate(req.getUri());
    if (routeTemplate.empty()) {
        return false;
    }

    const auto &routeInfo = routes.at(routeTemplate);
    if (routeInfo.method != req.getMethod()) {
        return false;
    }

    // Set route template parameters
    req.setRouteTemplateParams(routeTemplate, req.getUri());

    // Execute the route handler
    routeInfo.handler(req, res);
    return true;
}

void Router::printRoutes() const {
    std::cout << "Registered Routes:" << std::endl;
    for (const auto &route : routes) {
        std::cout << route.second.method << " " << route.first << std::endl;
    }
}