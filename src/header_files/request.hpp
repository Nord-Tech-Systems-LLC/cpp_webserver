#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <vector>
// #include <map>
#include <unordered_map>

#ifndef MAX_HTTP_HEADERS
#define MAX_HTTP_HEADERS 30
#endif

struct HttpHeader {
    std::string name;  // Header name
    std::string value; // Header value
};

class Request {
  private:
    // HttpMessage properties
    std::string method;              // HTTP method (GET, POST, etc.)
    std::string uri;                 // Requested URI
    std::string query;               // Query parameters as a single string
    std::string proto;               // HTTP version (e.g., HTTP/1.1)
    std::vector<HttpHeader> headers; // Headers as vector of HttpHeader
    std::string body;                // Body content
    std::string head;                // Request line and headers as a single string
    std::string message;             // Full request message (head + body)

    // Parsed query and path parameters for easier access
    std::unordered_map<std::string, std::string> queryParams; // example: ?userId=123&bookId=456
    std::unordered_map<std::string, std::string>
        routeTemplateParams; // example: /users/123/books/456

  public:
    // Getters
    std::string getMethod() const;
    std::string getUri() const;
    std::string getProto() const;
    std::string getQuery() const;
    std::vector<HttpHeader> getHeaders() const;
    std::string getBody() const;
    std::string getHead() const;
    std::string getMessage() const;
    std::unordered_map<std::string, std::string> getQueryParams() const;
    std::unordered_map<std::string, std::string> getRouteTemplateParams() const;

    // Setters
    void setMethod(const std::string &newMethod);
    void setUri(const std::string &newUri);
    void setProto(const std::string &newProto);
    void setHeaders(const std::vector<HttpHeader> &newHeaders);
    void setBody(const std::string &newBody);
    void setHead(const std::string &newHead);
    void setMessage(const std::string &newMessage);
    void setParams(const std::string &queryString);
    void setRouteTemplateParams(const std::string &routePattern, const std::string &requestUri);

    // Helper Methods
    std::string returnParamValue(const std::string &paramKey) const;
    std::string contentLength() const;
    std::vector<std::string> split_path(const std::string &path, char delimiter = '/');

    // Utility to get specific header value by name
    std::string getHeaderValue(const std::string &name) const;

    // Reset function
    void reset();
};

#endif