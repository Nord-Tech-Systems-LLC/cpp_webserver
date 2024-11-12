#include <string>
#include <unordered_map>
#include <map>
#include "cpp_webserver_include/core.hpp"

// getters
std::string Request::getMethod() const
{
    return method;
}

std::string Request::getPath() const
{
    return path;
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return headers;
}

std::string Request::getBody() const
{
    return body;
}

std::unordered_map<std::string, std::string> Request::getParams() const
{
    return queryParams;
}

// setters
void Request::setMethod(std::string newMethod)
{
    method = newMethod;
}

void Request::setPath(std::string newPath)
{
    path = newPath;
}

void Request::setHeaders(std::map<std::string, std::string> newHeaders)
{
    headers = newHeaders;
}

void Request::setBody(std::string newBody)
{
    body = newBody;
}

std::string Request::returnParamValue(std::string paramKey)
{
    return getParams()[paramKey];
}

void Request::setParams(std::string queryString)
{
    // reset map after each request
    queryParams.clear();

    // find start of query in queryString
    size_t queryPos = queryString.find('?');

    if (queryPos != std::string::npos)
    {
        // find 1 position after '?' character
        queryString = queryString.substr(queryPos + 1);

        size_t pos = 0;
        while (pos < queryString.length())
        {
            size_t delimPos = queryString.find('&', pos);
            std::string param;
            if (delimPos != std::string::npos)
            {
                param = queryString.substr(pos, delimPos - pos);
                pos = delimPos + 1;
            }
            else
            {
                param = queryString.substr(pos);
                pos = queryString.length();
            }
            size_t eqPos = param.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = param.substr(0, eqPos);
                std::string value = param.substr(eqPos + 1);
                // URL decoding might be required here depending on your use case
                queryParams[key] = value;
            }
        }
    }
}

// helper methods
std::string Request::contentLength(const std::string &input_body)
{
    // input html return content length
    return std::to_string(input_body.size());
};
