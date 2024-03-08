#include <unistd.h>

#include <string>

#include "hpp_files/http_server.hpp"
#include "hpp_files/server_logging.hpp"

/**
 * TODO:
 * - write dispatch events GET POST PUT DELETE
 * 
 */

int main() {
    HttpServer server("8080");

    // add additional routes
    server.getMethod("/custom", [](Request &httpRequest, Response &httpResponse) {
        // request
        httpRequest.setBody("<html><body>Hello!</body></html>");
        std::string request_body_count = httpRequest.contentLength(httpRequest.getBody());

        // response
        httpResponse.setBody("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + request_body_count + "\r\n\r\n" + httpRequest.getBody());

    });

    server.getMethod("/testing", [](Request &httpRequest, Response &httpResponse) {
        // request
        httpRequest.setBody("{\"message\": \"testing\"}");
        std::string request_body_count = httpRequest.contentLength(httpRequest.getBody());

        // response
        httpResponse.setBody("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + request_body_count + "\r\n\r\n" + httpRequest.getBody());

    });

    server.getMethod("/other", [](Request &httpRequest, Response &httpResponse) {
        // request
        httpRequest.setBody("<html><body>Page 3!</body></html>");
        std::string request_body_count = httpRequest.contentLength(httpRequest.getBody());

        // response
        httpResponse.setBody("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + request_body_count + "\r\n\r\n" + httpRequest.getBody());

    });

    server.printRoutes();

    server.start();

    return 0;
}
