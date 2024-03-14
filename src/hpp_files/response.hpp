#pragma once

#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <map>
#include <sstream>



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
        void setHeaders(std::map<std::string, std::string>newHeaders);
        void setBody(std::string newBody);
        void setRequestMethod(std::string newRequestMethod);

        // response router
        void GET(std::string responseContent);
        void PUT(std::string responseContent);
        void POST(std::string responseContent);
        void DELETE(std::string responseContent);

        // helper methods
        std::string contentLength(const std::string &input_body);
        std::string buildResponse(const std::string &responseStatus, const std::string &responseContent);
};

#endif