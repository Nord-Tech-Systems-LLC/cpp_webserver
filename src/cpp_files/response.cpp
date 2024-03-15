#include <string>
#include <map>
#include <sstream>
#include "../hpp_files/response.hpp"
#include "../hpp_files/server_logging.hpp"


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

void Response::setHeaders(std::map<std::string, std::string>newHeaders) {
    headers = newHeaders;
}

void Response::setBody(std::string newBody) {
    body = newBody;
}

void Response::setRequestMethod(std::string newRequestMethod) {
    requestMethod = newRequestMethod;
}

// response router
void Response::GET(std::string responseContent) {
    if (requestMethod != "GET") {
        logger::error("Requested: " + requestMethod + " / Expected: 'GET'" + " -- Improper request method");
        std::string response = buildResponse("400 Bad Request", "Improper request method.");
        body = response;
    } else {
        std::string response = buildResponse("200 OK", responseContent);
        body = response;
    }
}

void Response::PUT(std::string responseContent) {
    if (requestMethod != "PUT") {
        logger::error("Requested: " + requestMethod + " / Expected: 'PUT'" + " -- Improper request method");
        std::string response = buildResponse("400 Bad Request", "Improper request method.");
        body = response;
    } else {
        std::string response = buildResponse("200 OK", responseContent);
        body = response;
    }
}

void Response::POST(std::string responseContent) {
    if (requestMethod != "POST") {
        logger::error("Requested: " + requestMethod + " / Expected: 'POST'" + " -- Improper request method");
        std::string response = buildResponse("400 Bad Request", "Improper request method.");
        body = response;
    } else {
        std::string response = buildResponse("200 OK", responseContent);
        body = response;
    }
}

void Response::DELETE(std::string responseContent) {
    if (requestMethod != "DELETE") {
        logger::error("Requested: " + requestMethod + " / Expected: 'DELETE'" + " -- Improper request method");
        std::string response = buildResponse("400 Bad Request", "Improper request method.");
        body = response;
    } else {
        std::string response = buildResponse("200 OK", responseContent);
        body = response;
    }
}

// helper methods
std::string Response::contentLength(const std::string &input_body) {
    // input html return content length
    return std::to_string(input_body.size());
};

std::string Response::buildResponse(const std::string &responseStatus, const std::string &responseContent) {
    std::string responseLength = contentLength(responseContent);
    headers["Content-Length"] = responseLength;
    std::ostringstream response;
    
    // append HTTP headers
    response << "HTTP/1.1 " + responseStatus + "\r\n";
    for (const auto& header : headers) {
        response << header.first << ": " << header.second << "\r\n";
    }
    
    // add a blank line to separate headers from body
    response << "\r\n";
    response << responseContent;
    return response.str();
}
