#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <unistd.h>

class HandleHttpRequest {
public:
    void handleConnection(int client_socket, const char *htmlResponse) {
        std::string httpRequest = readRequest(client_socket);
        if (!httpRequest.empty()) {
            HttpRequest parsedRequest = parseHttpRequest(httpRequest);
            processRequest(client_socket, parsedRequest, htmlResponse);
        }
        close(client_socket);
    }

private:
    struct HttpRequest {
        std::string method;
        std::string path;
        std::unordered_map<std::string, std::string> headers;
        std::string body;
    };

    std::string readRequest(int client_socket) {
        std::string request;
        char buffer[1024];

        ssize_t bytesRead;
        while ((bytesRead = read(client_socket, buffer, sizeof(buffer))) > 0) {
            request.append(buffer, bytesRead);
            size_t endHeaders = request.find("\r\n\r\n");
            if (endHeaders != std::string::npos) {
                // Headers received
                break;
            }
        }

        return request;
    }

    HttpRequest parseHttpRequest(const std::string& request) {
        HttpRequest parsedRequest;

        std::istringstream requestStream(request);
        std::string line;

        // Parse the first line to get the method and path
        std::getline(requestStream, line);
        std::istringstream firstLineStream(line);
        firstLineStream >> parsedRequest.method >> parsedRequest.path;

        // Parse headers
        while (std::getline(requestStream, line) && !line.empty()) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 2);  // Skip ': ' after the header key
                parsedRequest.headers[key] = value;
            }
        }

        // Parse the message body if it exists
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != std::string::npos && bodyStart + 4 < request.length()) {
            parsedRequest.body = request.substr(bodyStart + 4);
        }

        return parsedRequest;
    }

    void processRequest(int client_socket, const HttpRequest& request, const char *htmlResponse) {
        // Process the request (you can replace this with your logic)
        std::cout << "Received Request:" << std::endl;
        std::cout << "Method: " << request.method << std::endl;
        std::cout << "Path: " << request.path << std::endl;

        std::cout << "Headers:" << std::endl;
        for (const auto& header : request.headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }

        std::cout << "Body: " << request.body << std::endl;

        // Send a simple response
        // const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
        write(client_socket, htmlResponse, strlen(htmlResponse));
    }
};

// int main() {
//     SimpleHttpServer httpServer;
//     int server_socket, client_socket;
//     // ... (initialize and bind server_socket, accept connections, etc.)

//     while (true) {
//         // Accept a connection
//         client_socket = accept(server_socket, nullptr, nullptr);
//         if (client_socket == -1) {
//             perror("Error accepting connection");
//             continue;
//         }

//         // Handle the connection
//         httpServer.handleConnection(client_socket);
//     }

//     // ... (close sockets, cleanup, etc.)
//     return 0;
// }
