#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

HttpRequest parseHttpRequest(const std::string& request) {
    HttpRequest parsedRequest;

    // Split the request into lines
    std::istringstream requestStream(request);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(requestStream, line)) {
        lines.push_back(line);
    }

    // Parse the first line to get the method and path
    if (!lines.empty()) {
        std::istringstream firstLineStream(lines[0]);
        firstLineStream >> parsedRequest.method >> parsedRequest.path;
    }

    // Parse headers
    for (std::size_t i = 1; i < lines.size(); ++i) {
        std::size_t pos = lines[i].find(':');
        if (pos != std::string::npos) {
            std::string key = lines[i].substr(0, pos);
            std::string value = lines[i].substr(pos + 2);  // Skip ': ' after the header key
            parsedRequest.headers[key] = value;
        }
    }

    // Parse the message body if it exists
    std::size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart != std::string::npos && bodyStart + 4 < request.length()) {
        parsedRequest.body = request.substr(bodyStart + 4);
    }

    return parsedRequest;
}