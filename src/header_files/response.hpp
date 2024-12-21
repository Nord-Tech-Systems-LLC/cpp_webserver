#ifndef RESPONSE_H
#define RESPONSE_H

#include <map>
#include <sstream>
#include <string>

class Response {
  private:
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string requestMethod;

  public:
    // getters
    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;
    std::string getRequestMethod() const;

    // setters
    void setStatusCode(int newStatusCode);
    void setStatusMessage(std::string newStatusMessage);
    void setHeaders(std::map<std::string, std::string> newHeaders);
    void setSingleHeader(const std::string &headerName, const std::string &headerValue);
    void setBody(std::string newBody);
    void setRequestMethod(std::string newRequestMethod);

    // helper methods
    std::string contentLength(const std::string &input_body);
    std::string buildResponse(const std::string &status,
                              const std::map<std::string, std::string> &headers,
                              const std::string &body);

    void send(const std::string &content);
    void json(const std::string &jsonResponse);
    Response &status(int code);

    // reset method
    void reset();
};

#endif