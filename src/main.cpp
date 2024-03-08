#include <unistd.h>

#include <string>

#include "hpp_files/http_server.hpp"
#include "hpp_files/server_logging.hpp"

/**
 * TODO:
 * - write dispatch events GET POST PUT DELETE
 * 
 */

std::string contentLength(const std::string &input_body) {
    // input html return content length
    return std::to_string(input_body.size());
}

int main() {
    HttpServer server("8080");

    // Add additional routes
    server.addRoute("/custom", [](Request &httpRequest, Response &httpResponse) {
        // buffer size
        char *buf[540];

        // request
        httpRequest.setBody("<html><body>Hello!</body></html>");
        std::string request_body_count = contentLength(httpRequest.getBody());

        // response
        httpResponse.setBody("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + request_body_count + "\r\n\r\n" + httpRequest.getBody());

    });

    server.addRoute("/testing", [](Request &httpRequest, Response &httpResponse) {
        // buffer size
        char *buf[540];

        // request
        httpRequest.setBody("{\"message\": \"testing\"}");
        std::string request_body_count = contentLength(httpRequest.getBody());

        // response
        httpResponse.setBody("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + request_body_count + "\r\n\r\n" + httpRequest.getBody());

    });

    server.addRoute("/other", [](Request &httpRequest, Response &httpResponse) {
        // buffer size
        char *buf[540];

        // request
        httpRequest.setBody("<html><body>Page 3!</body></html>");
        std::string request_body_count = contentLength(httpRequest.getBody());

        // response
        httpResponse.setBody("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + request_body_count + "\r\n\r\n" + httpRequest.getBody());

    });

    server.print_routes();

    server.start();

    return 0;
}
