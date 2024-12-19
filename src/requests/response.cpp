#include "cpp_webserver_include/core.hpp"
#include <map>
#include <sstream>
#include <string>

// getters
int Response::getStatusCode() const {
    return statusCode;
}

std::string Response::getStatusMessage() const {
    return statusMessage;
}

std::map<std::string, std::string> Response::getHeaders() const {
    return headers;
}

std::string Response::getBody() const {
    return body;
}

std::string Response::getRequestMethod() const {
    return requestMethod;
}

// setters
void Response::setStatusCode(int newStatusCode) {
    statusCode = newStatusCode;
}

void Response::setStatusMessage(std::string newStatusMessage) {
    statusMessage = newStatusMessage;
}

void Response::setHeaders(std::map<std::string, std::string> newHeaders) {
    headers = newHeaders;
}

void Response::setBody(std::string newBody) {
    body = newBody;
}

void Response::setRequestMethod(std::string newRequestMethod) {
    requestMethod = newRequestMethod;
}

// helper methods
std::string Response::contentLength(const std::string &input_body) {
    // input html return content length
    return std::to_string(input_body.size());
};

void Response::json(const std::string &jsonResponse) {
    setHeaders({{"Content-Type", "application/json"}, {"Access-Control-Allow-Origin", "*"}});
    send(jsonResponse);
}

void Response::send(const std::string &content) {
    // Logic to send plain text or HTML
    std::string response = buildResponse("200 OK", headers, content);
    body = response;
}

Response &Response::status(int code) {
    // Set HTTP status code
    statusCode = code;
    return *this;
}

std::string Response::buildResponse(const std::string &status,
                                    const std::map<std::string, std::string> &headers,
                                    const std::string &body) {
    // Start with the status line
    std::string response = "HTTP/1.1 " + status + "\r\n";

    // Append headers
    for (const auto &header : headers) {
        response += header.first + ": " + header.second + "\r\n";
    }

    // Content-Length is critical for HTTP/1.1 compliance
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";

    // End of headers section
    response += "\r\n";

    // Append the body
    response += body;

    return response;
}