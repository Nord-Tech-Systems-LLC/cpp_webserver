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
    setHeaders({{"Content-Type", "application/json"}});
    send(jsonResponse);
}

void Response::send(const std::string &content) {
    // Logic to send plain text or HTML
    std::string response = buildResponse("200 OK", content);
    body = response;
}

Response &Response::status(int code) {
    // Set HTTP status code
    statusCode = code;
    return *this;
}

std::string Response::buildResponse(const std::string &responseStatus,
                                    const std::string &responseContent) {
    std::string responseLength = contentLength(responseContent);
    headers["Content-Length"] = responseLength;
    headers["Access-Control-Allow-Origin"] = "*";
    std::ostringstream response;

    // append HTTP headers
    response << "HTTP/1.1 " + responseStatus + "\r\n";
    for (const auto &header : headers) {
        response << header.first << ": " << header.second << "\r\n";
    }

    // add a blank line to separate headers from body
    response << "\r\n";
    response << responseContent;
    return response.str();
}
