#ifndef ROUTER_H
#define ROUTER_H

#include "cpp_webserver_include/core.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using Handler = std::function<void(Request &, Response &)>;

struct Route {
    std::string method;
    std::string path;
    Handler handler;
};

class Router {
  public:
    void get(const std::string &path, Handler h) { add("GET", path, h); }
    void post(const std::string &path, Handler h) { add("POST", path, h); }
    void put(const std::string &path, Handler h) { add("PUT", path, h); }
    void del(const std::string &path, Handler h) { add("DELETE", path, h); }

    // Returns false if no route matched
    bool dispatch(Request &req, Response &res) const {
        for (auto &route : routes_)
            if (route.method == req.method && route.path == req.path) {
                route.handler(req, res);
                return true;
            }
        return false;
    }

    void printRoutes() const {
        for (auto &r : routes_) std::cout << r.method << " " << r.path << "\n";
    }

  private:
    std::vector<Route> routes_;
    void add(const std::string &m, const std::string &p, Handler h) { routes_.push_back({m, p, h}); }
};

#endif