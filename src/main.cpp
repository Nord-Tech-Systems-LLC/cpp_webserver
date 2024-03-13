#include <unistd.h>

#include <string>

#include "hpp_files/http_server.hpp"
#include "hpp_files/server_logging.hpp"

/**
 * TODO:
 * - write dispatch events GET POST PUT DELETE
 * - move parseHttpRequest to align with GET POST PUT DELETE. 
 * Should be able to print method on request not server startup
 * - parse query params
 * - parse body of request
 * - reset params at the beginning of each request
 * 
 */

int main() {
    HttpServer server("8080");

    // add additional routes
    server.addRoute("/Custom2", [](Request &httpRequest, Response &httpResponse) {

        // response
        std::string response = "<html><body>Hello!</body></html>";
        std::string response_body_count = httpResponse.contentLength(response);


        Router router;
        router.getMethod("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + response_body_count + "\r\n\r\n" + response);

    });

    server.addRoute("/testing", [](Request &httpRequest, Response &httpResponse) {
        // response
        std::string response ="{\"message\": \"testing\"}";
        std::string response_body_count = httpRequest.contentLength(response);

        Router router;
        router.getMethod("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + response_body_count + "\r\n\r\n" + response);

    });

    server.addRoute("/other", [](Request &httpRequest, Response &httpResponse) {
        // response
        std::string response ="<html><body>Page 3!</body></html>";
        std::string response_body_count = httpRequest.contentLength(response);

        Router router;
        router.getMethod("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + response_body_count + "\r\n\r\n" + response);

    });

    server.printRoutes();

    server.start();

    return 0;
}
