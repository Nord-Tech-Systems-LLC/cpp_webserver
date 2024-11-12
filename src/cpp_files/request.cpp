#include "cpp_webserver_include/core.hpp"
#include <sstream>

// Getters
std::string Request::getMethod() const { return method; }
std::string Request::getUri() const { return uri; }
std::string Request::getProto() const { return proto; }
std::string Request::getQuery() const { return query; }
std::vector<HttpHeader> Request::getHeaders() const { return headers; }
std::string Request::getBody() const { return body; }
std::string Request::getHead() const { return head; }
std::string Request::getMessage() const { return message; }
std::unordered_map<std::string, std::string> Request::getParams() const { return queryParams; }

// Setters
void Request::setMethod(const std::string &newMethod) { method = newMethod; }
void Request::setUri(const std::string &newUri) { uri = newUri; }
void Request::setProto(const std::string &newProto) { proto = newProto; }
void Request::setHeaders(const std::vector<HttpHeader> &newHeaders) { headers = newHeaders; }
void Request::setBody(const std::string &newBody) { body = newBody; }
void Request::setHead(const std::string &newHead) { head = newHead; }
void Request::setMessage(const std::string &newMessage) { message = newMessage; }

// Parses a query string and stores each parameter in queryParams
void Request::setParams(const std::string &queryString)
{
    queryParams.clear();
    std::istringstream stream(queryString);
    std::string pair;

    while (std::getline(stream, pair, '&'))
    {
        size_t pos = pair.find('=');
        if (pos != std::string::npos)
        {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            queryParams[key] = value;
        }
    }
}

// Helper Methods

// Returns the value of a specific query parameter by key
std::string Request::returnParamValue(const std::string &paramKey) const
{
    auto it = queryParams.find(paramKey);
    return (it != queryParams.end()) ? it->second : "";
}

// Calculates the content length of the body
std::string Request::contentLength() const
{
    return std::to_string(body.size());
}

// Utility to get specific header value by name
std::string Request::getHeaderValue(const std::string &name) const
{
    for (const auto &header : headers)
    {
        if (header.name == name)
        {
            return header.value;
        }
    }
    return "";
}
