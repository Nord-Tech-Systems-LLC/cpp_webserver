#ifndef RESPONSE_H
#define RESPONSE_H

#include <sstream>
#include <string>
#include <unordered_map>

struct Response {
    int status = 200;
    std::string body;
    std::unordered_map<std::string, std::string> headers;

    Response &setStatus(int s) {
        status = s;
        return *this;
    }
    Response &setBody(const std::string &b) {
        body = b;
        return *this;
    }
    Response &setHeader(const std::string &k, const std::string &v) {
        headers[k] = v;
        return *this;
    }

    std::string build() const {
        static const std::unordered_map<int, std::string> statusText = {{
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
        }};

        std::ostringstream out;
        out << "HTTP/1.1 " << status << " " << (statusText.count(status) ? statusText.at(status) : "Unknown") << "\r\n";

        for (auto &[k, v] : headers) out << k << ": " << v << "\r\n";

        out << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        return out.str();
    }
};

#endif