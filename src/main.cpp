#include "cpp_webserver_include/core.hpp"
#include "nlohmann/json.hpp"
#include <string>

// TODO:
// - bug with setting headers on request from main.cpp

int main() {

    // Create a JSON object
    nlohmann::json jsonObject = {{"name", "John Doe"},
                                 {"age", 30},
                                 {"isStudent", false},
                                 {"skills", {"C++", "Python", "Rust"}},
                                 {"address", {{"city", "New York"}, {"zip", "10001"}}}};

    // Convert the JSON object to a string
    std::string jsonString = jsonObject.dump(4); // Pretty-print with 4 spaces indentation

    // HttpServer server("127.0.0.1", "8080");

    HttpServer server("127.0.0.1", "8080");

    server.get("/", [](Request &req, Response &res) { res.send("Hello, world!"); });

    server.get("/json", [&](Request &req, Response &res) { res.json(jsonString); });

    server.post("/submit", [](Request &req, Response &res) {
        // Handle POST data
        res.status(201).send("Submitted!");
    });
    server.printRoutes();
    server.start();

    return 0;
}