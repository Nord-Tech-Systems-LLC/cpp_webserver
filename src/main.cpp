#include "cpp_webserver_include/core.hpp"
#include <iostream>
#include <string>

// TODO:
// - Check body params are working
// - modify query params and path params variables to be query params and template route variable
// names
// - Add status code lookup tables

// explanations:
// - Query Params -- add http://localhost:8080/json?test=testing&test2=testing
// - Query template route -- add http://localhost:8080/json/{template_route}/data
// - Fragments -- add http://localhost:8080/json#fragment -- is not sent to the server, it is
// primarily for the client browser

int main() {
    HttpServer server("127.0.0.1", "8080");

    server.get("/", [](Request &req, Response &res) { res.status(201).send("Hello, world!"); });

    server.get("/json/:page/:limit", [&](Request &req, Response &res) {
        try {
            // Validate the path parameters
            int page = std::stoi(req.getPathParams().at(":page"));
            int limit = std::stoi(req.getPathParams().at(":limit"));

            // Further validation on page and limit
            if (page <= 0) {
                throw std::invalid_argument("Invalid page value.");
            }
            if (limit <= 0) {
                throw std::invalid_argument("Invalid limit value.");
            }

            res.status(200).send("hello world");

        } catch (const std::invalid_argument &e) {
            res.status(400).send("Bad Request: " + std::string(e.what()));
        } catch (const std::exception &e) {
            res.status(500).send("Internal Server Error: " + std::string(e.what()));
        }
    });

    server.post("/submit", [](Request &req, Response &res) {
        // Handle POST data
        res.status(201).send("Submitted!");
    });
    server.printRoutes();
    server.start();

    return 0;
}