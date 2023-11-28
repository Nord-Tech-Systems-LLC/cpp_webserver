#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

class HttpServer {
   public:
    HttpServer(const char *port) : port(port), server_socket(0) {}

    void start() {
        if (createSocket() && bindSocket() && listenSocket()) {
            std::cout << "Server listening on port " << port << std::endl;
            acceptConnections();
        }
        close(server_socket);
    }

   private:
    const char *port;
    int server_socket;

    // Helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
        switch (sa->sa_family) {
            case AF_INET:
                inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
                break;

            case AF_INET6:
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
                break;

            default:
                strncpy(s, "Unknown AF", maxlen);
                return NULL;
        }

        return s;
    }

    bool createSocket() {
        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket creation failed");
            return false;
        }
        return true;
    }

    bool bindSocket() {
        struct sockaddr_in server_address;

        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(std::stoi(port));

        if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
            perror("Bind failed");
            return false;
        }

        return true;
    }

    bool listenSocket() {
        if (listen(server_socket, 10) == -1) {
            perror("Listen failed");
            return false;
        }
        return true;
    }

    void handleRequest(int client_socket) {
        char buffer[1024] = {0};
        read(client_socket, buffer, sizeof(buffer));

        // Simple response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
        write(client_socket, response, strlen(response));

        close(client_socket);
    }

    void acceptConnections() {
        struct sockaddr_storage client_address;
        socklen_t client_address_len = sizeof(client_address);

        while (true) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
            if (client_socket == -1) {
                perror("Accept failed");
                break;
            }

            handleRequest(client_socket);
        }
    }
};

int main() {
    HttpServer server("8080");
    server.start();

    return 0;
}
