#pragma once

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <cstring>
#include <map>
#include <functional>
#include <string>
#include <unordered_map>


class Router {
    Response response;
    public:
        // response router
        void getMethod(std::string responseContent) {
            response.setBody(responseContent);
        }

        void putMethod(std::string responseContent) {
            response.setBody(responseContent);
        }

        void postMethod(std::string responseContent) {
            response.setBody(responseContent);
        }

        void deleteMethod(std::string responseContent) {
            response.setBody(responseContent);
        }
};

class Request {
    private:
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        std::unordered_map<std::string, std::string> queryParams;

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

        std::unordered_map<std::string, std::string> getParams() const {
            return queryParams;
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

        void setParams(std::string queryString) {
            // find start of query in queryString
            size_t queryPos = queryString.find('?');

            if (queryPos != std::string::npos) {
                // find 1 position after '?' character
                queryString = queryString.substr(queryPos + 1);

                size_t pos = 0;
                while (pos < queryString.length()) {
                    size_t delimPos = queryString.find('&', pos);
                    std::string param;
                    if (delimPos != std::string::npos) {
                        param = queryString.substr(pos, delimPos - pos);
                        pos = delimPos + 1;
                    } else {
                        param = queryString.substr(pos);
                        pos = queryString.length();
                    }
                    size_t eqPos = param.find('=');
                    if (eqPos != std::string::npos) {
                        std::string key = param.substr(0, eqPos);
                        std::string value = param.substr(eqPos + 1);
                        // URL decoding might be required here depending on your use case
                        queryParams[key] = value;
                    }
                }
            }
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
    void addRoute(const std::string &path, std::function<void(Request&, Response&)> handler);

   private:
    const char *port;
    int server_socket;

    // helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
    char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen);
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    void handleRequest(int client_socket);
    void handleResponse(int client_socket);
    void acceptConnections();

    // request & response
    Request httpRequest;
    Response httpResponse;
    bool checkRoutes();
    std::map<std::string, std::function<void(Request&, Response&)>> routes;

    // map to store method, route, and http version
    void parseHttpRequest(const std::string &requestBuffer);
};

#endif