#include "cpp_webserver_include/core.hpp"
#include <iostream>

// TODO:
// - Create cookie functionality

// explanations:
// - Query Params -- add http://localhost:8080/json?test=testing&test2=testing
// - Query template route -- add http://localhost:8080/json/{template_route}/data
// - Fragments -- add http://localhost:8080/json#fragment -- is not sent to the server, it is
// primarily for the client browser

class Users {
  public:
    static void getUserListData(Request &req, Response &res);
    static void createUser(Request &req, Response &res);
    static void getCurrentUser(Request &req, Response &res);
};

void Users::getUserListData(Request &req, Response &res) {
    try {
        int offset = std::stoi(req.query.at(":offset"));
        int limit = std::stoi(req.query.at(":limit"));

        res.setStatus(200).setBody("User List Data");
    } catch (const std::invalid_argument &e) {
        res.setStatus(400).setBody("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) { res.setStatus(500).setBody("Internal Server Error: " + std::string(e.what())); }
}

void Users::createUser(Request &req, Response &res) {
    try {
        res.setStatus(200).setBody(req.body);
    } catch (const std::invalid_argument &e) {
        res.setStatus(400).setBody("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) { res.setStatus(500).setBody("internal Server Error: " + std::string(e.what())); }
}

void Users::getCurrentUser(Request &req, Response &res) {
    try {
        res.setStatus(200).setBody("Current User!");
    } catch (const std::invalid_argument &e) {
        res.setStatus(400).setBody("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) { res.setStatus(500).setBody("Internal Server Error: " + std::string(e.what())); }
}

void user_controller(HttpServer &server) {
    server.get("/list_users/:offset/:limit", Users::getUserListData);
    server.get("/current_user", Users::getCurrentUser);
    server.post("/create_user", Users::createUser);
}

int main() {
    HttpServer server("127.0.0.1", "8080");
    // server.middleware({{"Access-Control-Allow-Origin", "*"}}); // set global header config
    server.use([](Request &req, Response &res, Next next) {
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

        if (req.method == "OPTIONS") {
            res.setStatus(204);
            return;
        }
    });
    user_controller(server); // load route controller
    server.printRoutes();
    server.start(); // start server
    return 0;
}