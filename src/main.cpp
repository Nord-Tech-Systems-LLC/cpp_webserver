#include <unistd.h>

#include "cpp_webserver/http_server.hpp"
#include "cpp_webserver/server_logging.hpp"

int main() {
    HttpServer server("8080");

    // Add additional routes
    server.addRoute("/custom", [](int client_socket) {
        std::cout.flush();
        // Custom response for the /custom path
        const char *htmlResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\n<html><body>Hello!</body></html>";
        // const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nCustom Route!";
        // logger::log(htmlResponse);
        logger::log("Expected Response: ", htmlResponse);
        // write(client_socket, htmlResponse, strlen(htmlResponse));
        ssize_t bytes_written = write(client_socket, htmlResponse, strlen(htmlResponse));
        if (bytes_written == -1) {
            perror("Error writing to client socket");
        }
        // close(client_socket);
    });

    server.addRoute("/testing", [](int client_socket) {
        std::cout.flush();
        // Custom response for the /other path
        const char *otherHtmlResponse = "HTTP/1.1 200 OK\r\nContent-Length: 90\r\n\r\n<html><body>This was other!</body></html>";
        // const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nCustom Route!";
        logger::log("Expected Response: ", otherHtmlResponse);
        ssize_t bytes_written = write(client_socket, otherHtmlResponse, strlen(otherHtmlResponse));
        if (bytes_written == -1) {
            perror("Error writing to client socket");
        }
        
        // write(client_socket, otherHtmlResponse, strlen(otherHtmlResponse));
        // close(client_socket);
    });

    server.start();

    return 0;
}
