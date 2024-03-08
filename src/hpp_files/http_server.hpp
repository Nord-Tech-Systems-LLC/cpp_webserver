#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <cstring>
#include <map>
#include <functional>
#include <string>

class Request {
    private:
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
    public:
        // getters
        std::string getMethod() const {
            return method;
        }

        std::string getPath() const {
            return path;
        }

        std::map<std::string, std::string> getHeaders() const {
            return headers;
        }

        std::string getBody() const {
            return body;
        }

        // setters
        void setMethod(std::string newMethod) {
            method = newMethod;
        }

        void setPath(std::string newPath) {
            path = newPath;
        }

        void setHeaders(std::map<std::string, std::string> newHeaders) {
            headers = newHeaders;
        }

        void setBody(std::string newBody) {
            body = newBody;
        }

        // helper methods
        std::string contentLength(const std::string &input_body) {
            // input html return content length
            return std::to_string(input_body.size());
        };

};

class Response {
    private:
        int statusCode;
        std::string statusMessage;
        std::map<std::string, std::string> headers;
        std::string body;

    public:
        // getters 
        int getStatusCode() const {
            return statusCode;
        }

        std::string getStatusMessage() const {
            return statusMessage;
        }

        std::map<std::string, std::string> getHeaders() const {
            return headers;
        }

        std::string getBody() const {
            return body;
        }

        // setters
        void setStatusCode(int newStatusCode) {
            statusCode = newStatusCode;
        }

        void setStatusMessage(std::string newStatusMessage) {
            statusMessage = newStatusMessage;
        }

        void setHeaders(std::map<std::string, std::string>newHeaders) {
            headers = newHeaders;
        }

        void setBody(std::string newBody) {
            body = newBody;
        }

        // helper methods
        std::string contentLength(const std::string &input_body) {
            // input html return content length
            return std::to_string(input_body.size());
        };
};

class HttpServer {
   public:
    HttpServer(const char *port);
    // ~HttpServer();
    void start();
    void printRoutes();

    // dispatch events
    void getMethod(const std::string &path, std::function<void(Request&, Response&)> handler);
    void postMethod(const std::string &path, std::function<void(Request&, Response&)> handler);
    void putMethod(const std::string &path, std::function<void(Request&, Response&)> handler);
    void deleteMethod(const std::string &path, std::function<void(Request&, Response&)> handler);

   private:
    const char *port;
    int server_socket;

    // helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    void handleRequest(int client_socket);
    void acceptConnections();

    // request & response
    Request httpRequest;
    Response httpResponse;
    bool checkRoutes(const std::string& route_request);
    std::map<std::string, std::function<void(Request&, Response&)>> routes;

    // map to store method, route, and http version
    std::map<std::string, std::string> parsedInfo;
    std::map<std::string, std::string> parseHttpRequest(const std::string &requestBuffer);
};

#endif