#ifndef RESPONSE_H
#define RESPONSE_H

#include <map>
#include <sstream>
#include <string>

class Response {
  private:
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string requestMethod;
    bool sent = false;

    // RFC 2616 §10 — status code registry
    const std::map<int, std::string> http_status_codes = {
        {100, "Continue"},
        {101, "Switching Protocols"},
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {204, "No Content"},
        {206, "Partial Content"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {304, "Not Modified"},
        {307, "Temporary Redirect"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {413, "Request Entity Too Large"},
        {414, "Request-URI Too Long"},
        {415, "Unsupported Media Type"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {505, "HTTP Version Not Supported"},
    };

  public:
    // getters
    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;
    std::string getRequestMethod() const;
    bool isSent() const;

    // setters
    void setStatusCode(int newStatusCode);
    void setStatusMessage(std::string newStatusMessage);
    void setHeaders(std::map<std::string, std::string> newHeaders);
    void setSingleHeader(const std::string &headerName, const std::string &headerValue);
    void setBody(std::string newBody);
    void setRequestMethod(std::string newRequestMethod);

    // helper methods
    std::string contentLength(const std::string &input_body);
    std::string buildResponse(const std::string &status, const std::map<std::string, std::string> &headers,
                              const std::string &body);

    void send(const std::string &content);
    void json(const std::string &jsonResponse);
    Response &status(int code);

    // reset method
    void reset();
};

#endif