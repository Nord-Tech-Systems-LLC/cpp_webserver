#include "cpp_webserver_include/core.hpp"
#include <string>

// TODO:
// - bug with setting headers on request from main.cpp

int main() {
    // HttpServer server("127.0.0.1", "8080");

    HttpServer server("127.0.0.1", "8080");

    server.get("/", [](Request &req, Response &res) { res.send("Hello, world!"); });

    server.get(
        "/json", [](Request &req, Response &res) { res.json("{\"message\": \"Hello, JSON\"}"); });

    server.post("/submit", [](Request &req, Response &res) {
        // Handle POST data
        res.status(201).send("Submitted!");
    });
    server.printRoutes();
    server.start();

    return 0;
}