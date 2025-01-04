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

class Request {
  public:
    // httpMessage properties
    std::string method;  // HTTP method (GET, POST, etc.)
    std::string uri;     // requested URI
    std::string query;   // query parameters as a single string
    std::string proto;   // HTTP version (e.g., HTTP/1.1)
    std::string body;    // body content
    std::string message; // full request message (head + body)

    // parsed query and path parameters for easier access
    std::unordered_map<std::string, std::string> headers;     // headers as vector of HttpHeader
    std::unordered_map<std::string, std::string> queryParams; // ex: ?userId=123&bookId=456
    std::unordered_map<std::string, std::string> routeTemplateParams; // ex: /users/123/books/456
    std::unordered_map<std::string, std::string> cookies;

    // build request for server
    void buildRequest(std::string &message, Router &router);

    // utility to get specific header value by name
    std::string getHeaderValue(const std::string &name) const;

    // reset function
    void reset();

  private:
    // setters
    void setMethod(const std::string &newMethod);
    void setUri(const std::string &newUri);
    void setProto(const std::string &newProto);
    void setHeaders(const std::unordered_map<std::string, std::string> &newHeaders);
    void setBody(const std::string &newBody);
    void setMessage(const std::string &newMessage);
    void setParams(const std::string &uri);
    void setRouteTemplateParams(const std::string &routePattern, const std::string &requestUri);
    void setSingleCookie(const std::string &cookieName, const std::string &cookieValue);
    void parseCookies(const std::unordered_map<std::string, std::string> &headers);

    // helpers
    std::unordered_map<std::string, std::string> extractHttpHeader(const std::string &message);
    std::string extractMainRoute(const std::string &url);
    std::string contentLength() const;
    std::vector<std::string> split_path(const std::string &path, char delimiter = '/');
};

#endif