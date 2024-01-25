#include <unistd.h>

#include <string>

#include "cpp_webserver/http_server.hpp"
#include "cpp_webserver/server_logging.hpp"
#include "http_request.cpp"

/**
 * TODO:
 * - When routes consist of a number first i.e. /2 -- route gets stuck and can't move
 * - Route matching needs to be revised to exact instead of relative i.e. /other and /other2 returns /other route
 */

std::string contentLength(std::string &input_body) {
    // input html return content length
    return std::to_string(input_body.size());
}

int main() {
    HttpServer server("8080");

    // Add additional routes
    server.addRoute("/custom", [](int client_socket) {
        // buffer size
        char *buf[540];
        // response
        std::string body = "<html><body>Hello!</body></html>";
        std::string body_count = contentLength(body);

        // make string to combine content-length
        std::string string_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + body_count + "\r\n\r\n" + body;
        const char *htmlResponse = string_response.c_str();

        // logger::log(htmlResponse);

        // while bytes_written is less than byte_count_transfer
        int byte_count_transfer = 0;
        ssize_t bytes_written = write(client_socket, htmlResponse, strlen(htmlResponse));
        do {
            byte_count_transfer++;
        } while (byte_count_transfer <= bytes_written);
        close(client_socket);
    });

    server.addRoute("/testing", [](int client_socket) {
        // buffer size
        char *buf[540];

        // response
        std::string body = "{\"message\": \"testing\"}";
        std::string body_count = contentLength(body);

        // make string to combine content-length
        std::string string_response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + body_count + "\r\n\r\n" + body;
        const char *htmlResponse = string_response.c_str();

        // logger::log(htmlResponse);

        // while bytes_written is less than byte_count_transfer
        int byte_count_transfer = 0;
        ssize_t bytes_written = write(client_socket, htmlResponse, strlen(htmlResponse));
        do {
            byte_count_transfer++;
        } while (byte_count_transfer <= bytes_written);
        close(client_socket);
    });

    server.addRoute("/other", [](int client_socket) {
        // buffer size
        char *buf[540];

        // response
        std::string body = "<html><body>Page 3!</body></html>";
        std::string body_count = contentLength(body);

        // make string to combine content-length
        std::string string_response = "HTTP/1.1 200 OK\r\nContent-Length: " + body_count + "\r\n\r\n" + body;
        const char *htmlResponse = string_response.c_str();

        // logger::log(htmlResponse);

        // while bytes_written is less than byte_count_transfer
        int byte_count_transfer = 0;
        ssize_t bytes_written = write(client_socket, htmlResponse, strlen(htmlResponse));
        do {
            byte_count_transfer++;
        } while (byte_count_transfer <= bytes_written);
        close(client_socket);
    });

    server.start();

    return 0;
}
