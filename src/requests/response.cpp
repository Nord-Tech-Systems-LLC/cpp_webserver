#include "cpp_webserver_include/core.hpp"
#include <map>
#include <sstream>
#include <string>

// getters
int Response::getStatusCode() const {
    return status_code;
}

std::string Response::getStatusMessage() const {
    return status_message;
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
    status_code = newStatusCode;
}

void Response::setStatusMessage(std::string newStatusMessage) {
    status_message = newStatusMessage;
}

void Response::setSingleHeader(const std::string &headerName, const std::string &headerValue) {
    // Set the single header in the map
    headers[headerName] = headerValue;
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

bool Response::isSent() const {
    return sent;
}

// helper methods
std::string Response::contentLength(const std::string &input_body) {
    // input html return content length
    return std::to_string(input_body.size());
};

void Response::json(const std::string &jsonResponse) {
    setSingleHeader("Content-Type", "application/json");
    send(jsonResponse);
}

void Response::send(const std::string &content) {
    // Logic to send plain text or HTML
    std::string response = buildResponse(
        std::to_string(status_code) + " " + std::string(status_message), headers, content);
    body = response;
    sent = true;
}

Response &Response::status(int code) {
    // Set HTTP status code
    status_code = code;

    // Find the corresponding status message
    auto it = http_status_codes.find(code);
    if (it != http_status_codes.end()) {
        status_message = it->second; // Set the message if found
    } else {
        throw std::invalid_argument("Invalid HTTP status code.");
    }

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

void Response::reset() {
    sent = false;           // reset to notify if res is sent
    status_code = 0;        // Reset status code to a default value
    status_message.clear(); // Clear status message
    headers.clear();        // Clear all headers
    body.clear();           // Clear response body
    requestMethod.clear();  // Clear request method
}