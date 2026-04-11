
#ifndef REQUEST_H
#define REQUEST_H

#include <sstream>
#include <string>
#include <unordered_map>

struct Request {
    std::string method;
    std::string path; // ← clean path, no query string
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query; // ← new

    static Request parse(const std::string &raw) {
        Request req;
        std::istringstream stream(raw);
        std::string line;

        std::getline(stream, line);
        std::istringstream reqLine(line);
        std::string fullPath, version;
        reqLine >> req.method >> fullPath >> version;

        // ── Split path and query string
        auto qmark = fullPath.find('?');
        if (qmark != std::string::npos) {
            req.path = fullPath.substr(0, qmark);
            parseQuery(req.query, fullPath.substr(qmark + 1));
        } else {
            req.path = fullPath;
        }

        // ── Headers
        while (std::getline(stream, line) && line != "\r") {
            auto colon = line.find(':');
            if (colon == std::string::npos) continue;
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);
            value.erase(value.find_last_not_of("\r\n") + 1);
            req.headers[key] = value;
        }

        // ── Body
        if (req.headers.count("Content-Length")) {
            int len = std::stoi(req.headers["Content-Length"]);
            req.body.resize(len);
            stream.read(req.body.data(), len);
        }

        return req;
    }

  private:
    // Parses "foo=bar&baz=qux" into the query map
    static void parseQuery(std::unordered_map<std::string, std::string> &out, const std::string &qs) {
        std::istringstream stream(qs);
        std::string pair;
        while (std::getline(stream, pair, '&')) {
            auto eq = pair.find('=');
            if (eq == std::string::npos) {
                out[pair] = ""; // key with no value
            } else {
                out[pair.substr(0, eq)] = pair.substr(eq + 1);
            }
        }
    }
};

#endif