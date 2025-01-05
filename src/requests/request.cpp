#include "cpp_webserver_include/core.hpp"
#include <sstream>
#include <stdexcept>

// Setters
void Request::setMethod(const std::string &newMethod) {
    method = newMethod;
}
void Request::setUri(const std::string &newUri) {
    uri = newUri;
}
void Request::setProto(const std::string &newProto) {
    proto = newProto;
}
void Request::setHeaders(const std::unordered_map<std::string, std::string> &newHeaders) {
    headers = newHeaders;
}
void Request::setBody(const std::string &newBody) {
    body = newBody;
}
void Request::setMessage(const std::string &newMessage) {
    message = newMessage;
}
void Request::setSingleCookie(const std::string &cookieName, const std::string &cookieValue) {
    // Set the single header in the map
    cookies[cookieName] = cookieValue;
}

// Parses a query string and stores each parameter in queryParams
void Request::setParams(const std::string &uri) {
    queryParams.clear();

    // Find the query string (after the '?')
    size_t questionMarkPos = uri.find('?');
    if (questionMarkPos == std::string::npos || questionMarkPos + 1 >= uri.size()) {
        return; // No query string to parse or query string is empty
    }

    std::string queryString = uri.substr(questionMarkPos + 1);

    std::istringstream stream(queryString);
    std::string pair;

    // Parse key-value pairs
    while (std::getline(stream, pair, '&')) {
        if (pair.empty())
            continue; // Skip empty pairs

        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            // Key and value present
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            queryParams[key] = value;
        } else {
            // Handle keys with no values (e.g., "key&key2=value2")
            std::string key = pair;
            queryParams[key] = "";
        }
    }
}

// Parses a query string and stores each parameter in queryParams
void Request::setRouteTemplateParams(const std::string &routePattern,
                                     const std::string &requestUri) {
    routeTemplateParams.clear();

    // Split the route pattern and request URI into segments
    auto routeSegments = split_path(routePattern);
    auto requestSegments = split_path(requestUri);

    // If the number of segments doesn't match, the route doesn't match
    if (routeSegments.size() != requestSegments.size()) {
        return;
    }
    // Check each segment for a match
    for (size_t i = 0; i < routeSegments.size(); ++i) {
        if (routeSegments[i].front() == ':') {
            // If the route segment starts with ':', treat it as a wildcard
            routeTemplateParams[routeSegments[i]] = requestSegments[i];
            continue;
        } else if (routeSegments[i] != requestSegments[i]) {
            // If the segments don't match, return false
            return;
        }
    }
}

void Request::parseCookies(const std::unordered_map<std::string, std::string> &headers) {
    auto it = headers.find("Cookie");
    if (it != headers.end()) {
        std::istringstream cookieStream(it->second);
        std::string cookiePair;
        while (std::getline(cookieStream, cookiePair, ';')) {
            size_t eq_pos = cookiePair.find('=');
            if (eq_pos != std::string::npos) {
                std::string name = cookiePair.substr(0, eq_pos);
                std::string value = cookiePair.substr(eq_pos + 1);
                trim(name);                   // Remove leading/trailing whitespace
                trim(value);                  // Trim spaces around values
                setSingleCookie(name, value); // Set cookie in the request object
            }
        }
    }
}

void Request::buildRequest(std::string &message, Router &router) {
    // set main message of request for building
    setMessage(message);

    // find the body of the request
    size_t body_start = message.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        // the body starts after the "\r\n\r\n" sequence
        std::string body = message.substr(body_start + 4);
        // set the body in the HttpRequest object
        setBody(body);
    } else {
        // if there's no body in the request, set it to an empty string
        setBody("");
    }

    // extract the method, URI, and HTTP version (can be extracted from the message directly)
    size_t method_end = message.find(" ");
    size_t uri_end = message.find(" ", method_end + 1);
    size_t proto_end = message.find("\r\n", uri_end + 1);

    // extract method, URI, and HTTP version
    std::string method = message.substr(0, method_end);
    std::string uri = message.substr(method_end + 1, uri_end - method_end - 1);
    std::string proto = message.substr(uri_end + 1, proto_end - uri_end - 1);

    // set the values in the HttpRequest object
    setMethod(method);                      // main request method
    setUri(uri);                            // uri route
    setProto(proto);                        // protocol
    setHeaders(extractHttpHeader(message)); // setting headers after each request
    setParams(uri);                         // parse url params and set them for the request
    parseCookies(headers);                  // setting cookies from headers
    setUri(extractMainRoute(uri));          // setting main route for lookup

    // Set route template parameters
    std::string routeTemplate = router.findMatchingRouteTemplate(uri);
    setRouteTemplateParams(routeTemplate, uri);
};

// helper Methods
std::unordered_map<std::string, std::string>
Request::extractHttpHeader(const std::string &message) {
    std::unordered_map<std::string, std::string> headerMap;
    std::istringstream stream(message);
    std::string line;

    // Skip the request line (first line)
    std::getline(stream, line);

    // Loop through each subsequent line until we find an empty line
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            trim(name);  // Remove whitespace around the header name
            trim(value); // Remove leading/trailing whitespace around the header value

            headerMap[name] = value; // Insert into the map
        }
    }
    return headerMap;
};

std::string Request::extractMainRoute(const std::string &url) {
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        return url.substr(0, queryPos);
    } else {
        return url;
    }
};

// Calculates the content length of the body
std::string Request::contentLength() const {
    return std::to_string(body.size());
}

// Utility to get specific header value by name
std::string Request::getHeaderValue(const std::string &name) const {
    auto it = headers.find(name);
    if (it != headers.end()) {
        return it->second; // Return the value associated with the name
    }
    return ""; // Return empty string if the header is not found
}

// Split a string into segments by a delimiter
std::vector<std::string> Request::split_path(const std::string &path, char delimiter) {
    std::vector<std::string> segments;
    std::stringstream ss(path);
    std::string segment;

    while (std::getline(ss, segment, delimiter)) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }

    return segments;
}

// Reset function implementation
void Request::reset() {
    cookies.clear();
    method.clear();
    uri.clear();
    query.clear();
    proto.clear();
    headers.clear();
    body.clear();
    message.clear();
    queryParams.clear();
    routeTemplateParams.clear();
}
