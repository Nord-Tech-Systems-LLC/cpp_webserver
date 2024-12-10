#include "cpp_webserver_include/core.hpp"
#include <iostream>
#include <string>

// TODO:
// - Check body params are working

int main() {
    HttpServer server("127.0.0.1", "8080");

    server.get("/", [](Request &req, Response &res) { res.send("Hello, world!"); });

    server.get("/json/:userId/user/:bookId", [](Request &req, Response &res) {
        std::string userId = req.getPathParams().at(":userid");
        std::string bookId = req.getPathParams().at(":bookid");

        // Construct a JSON response string
        std::string json = "{\"userId\": \"" + userId + "\", \"bookId\": \"" + bookId + "\"}";

        res.json(json);
    });

    server.post("/submit", [](Request &req, Response &res) {
        // Handle POST data
        res.status(201).send("Submitted!");
    });
    server.printRoutes();
    server.start();

    return 0;
}