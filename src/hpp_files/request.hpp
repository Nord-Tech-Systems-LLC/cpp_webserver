#pragma once

#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <unordered_map>
#include <map>

class Request {
    private:
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        std::unordered_map<std::string, std::string> queryParams;

    public:
        // getters
        std::string getMethod() const;
        std::string getPath() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::unordered_map<std::string, std::string> getParams() const;

        // setters
        void setMethod(std::string newMethod);
        void setPath(std::string newPath);
        void setHeaders(std::map<std::string, std::string> newHeaders);
        void setBody(std::string newBody);
        void setParams(std::string queryString);

        // helper methods
        std::string contentLength(const std::string &input_body);
};

#endif