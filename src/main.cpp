#include <unistd.h>

#include "cpp_webserver/http_server.hpp"

int main() {
    HttpServer server("8080");

    // Add additional routes
    server.addRoute("/custom", [](int client_socket) {
        // Custom response for the /custom path
        const char *htmlResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\n<html><body>Hello!</body></html>";
        // const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nCustom Route!";
        write(client_socket, htmlResponse, strlen(htmlResponse));
    });

    server.addRoute("/other", [](int client_socket) {
        // Custom response for the /other path
        const char *htmlResponse = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 26\r\n\r\n{\"message\":\"Hello!\"}";
        // const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nCustom Route!";
        // ssize_t bytes_written = write(client_socket, htmlResponse, strlen(htmlResponse));
        // if (bytes_written == -1) {
        //     perror("Error writing to client socket");
        // }
        write(client_socket, htmlResponse, strlen(htmlResponse));
    });

    server.start();

    return 0;
}
