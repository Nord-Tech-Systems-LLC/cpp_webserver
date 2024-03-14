#include <unistd.h>

#include <string>

#include "hpp_files/http_server.hpp"
#include "hpp_files/server_logging.hpp"

/**
 * TODO:
 * - write dispatch events GET POST PUT DELETE
 * - parse query params
 * - parse body of request
 * - set headers on back end
 * 
 * 
 */

int main() {
    HttpServer server("8080");

    // add additional routes
    server.addRoute("/Custom2", [](Request &httpRequest, Response &httpResponse) {
        // set headers
        httpResponse.setHeaders({
            {"Content-Type", "text/html"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });

        // response
        std::string response = "<html><body>Hello!</body></html>";
        httpResponse.GET(response);
    });

    server.addRoute("/testing", [](Request &httpRequest, Response &httpResponse) {
        // set headers
        httpResponse.setHeaders({
            {"Content-Type", "application/json"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });

        // response
        std::string response ="{\"message\": \"testing\"}";
        httpResponse.GET(response);
    });

    server.addRoute("/other", [](Request &httpRequest, Response &httpResponse) {
        // set headers
        httpResponse.setHeaders({
            {"Content-Type", "text/html"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });

        // response
        std::string response ="<html><body>Page 3!</body></html>";
        httpResponse.GET(response);

    });

    server.printRoutes();

    server.start();

    return 0;
}
