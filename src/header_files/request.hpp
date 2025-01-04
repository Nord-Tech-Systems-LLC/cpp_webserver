#ifndef REQUEST_H
#define REQUEST_H

#include "cpp_webserver_include/core.hpp"

#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef MAX_HTTP_HEADERS
#define MAX_HTTP_HEADERS 30
#endif

struct HttpHeader {
    std::string name;  // header name
    std::string value; // header value
};

class Request {
  public:
    void buildRequest(std::string &message, Router &router);
    // httpMessage properties
    std::string method;              // HTTP method (GET, POST, etc.)
    std::string uri;                 // Requested URI
    std::string query;               // Query parameters as a single string
    std::string proto;               // HTTP version (e.g., HTTP/1.1)
    std::vector<HttpHeader> headers; // Headers as vector of HttpHeader
    std::string body;                // Body content
    std::string message;             // Full request message (head + body)
    std::unordered_map<std::string, std::string> cookies;

    // parsed query and path parameters for easier access
    std::unordered_map<std::string, std::string> queryParams; // example: ?userId=123&bookId=456
    std::unordered_map<std::string, std::string>
        routeTemplateParams; // example: /users/123/books/456

    // helper Methods
    std::string returnParamValue(const std::string &paramKey) const;
    std::string contentLength() const;
    std::vector<std::string> split_path(const std::string &path, char delimiter = '/');

    // utility to get specific header value by name
    std::string getHeaderValue(const std::string &name) const;

    // reset function
    void reset();

  private:
    // setters
    void setMethod(const std::string &newMethod);
    void setUri(const std::string &newUri);
    void setProto(const std::string &newProto);
    void setHeaders(const std::vector<HttpHeader> &newHeaders);
    void setBody(const std::string &newBody);
    void setMessage(const std::string &newMessage);
    void setParams(const std::string &uri);
    void setRouteTemplateParams(const std::string &routePattern, const std::string &requestUri);
    void setSingleCookie(const std::string &cookieName, const std::string &cookieValue);
    void parseCookies(const std::vector<HttpHeader> &headers);

    // helpers
    std::vector<HttpHeader> extractHttpHeader(const std::string &message);
    std::string extractMainRoute(const std::string &url);
};

#endif