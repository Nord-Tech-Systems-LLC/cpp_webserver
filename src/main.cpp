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
        // int offset = std::stoi(req.routeTemplateParams.at(":offset"));
        // int limit = std::stoi(req.routeTemplateParams.at(":limit"));

        res.status(200).send("User List Data");
    } catch (const std::invalid_argument &e) {
        res.status(400).send("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) {
        res.status(500).send("Internal Server Error: " + std::string(e.what()));
    }
}

void Users::createUser(Request &req, Response &res) {
    try {
        res.status(200).send(req.body);
    } catch (const std::invalid_argument &e) {
        res.status(400).send("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) {
        res.status(500).send("internal Server Error: " + std::string(e.what()));
    }
}

void Users::getCurrentUser(Request &req, Response &res) {
    try {
        // std::cout << "Cookie_10" << req.cookies.at("Cookie_10") << std::endl;
        res.status(200).send("Current User!");
    } catch (const std::invalid_argument &e) {
        res.status(400).send("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) {
        res.status(500).send("Internal Server Error: " + std::string(e.what()));
    }
}

void user_controller(HttpServer &server) {
    server.get("/list_users/:offset/:limit", Users::getUserListData);
    server.get("/current_user", Users::getCurrentUser);
    server.post("/create_user", Users::createUser);
}

int main() {
    HttpServer server("127.0.0.1", "8080");
    server.middleware({{"Access-Control-Allow-Origin", "*"}}); // set global header config
    server.use(                                                // set middleware
        [](Request &req, Response &res) { std::cout << "This is middleware!" << std::endl; });
    user_controller(server); // load route controller
    server.printRoutes();
    server.start(); // start server
    return 0;
}