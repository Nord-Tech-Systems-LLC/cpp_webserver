#include "cpp_webserver_include/core.hpp"
#include <sstream>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
//  Setters
// ─────────────────────────────────────────────────────────────────────────────

void Request::setMethod(const std::string &v) {
    method = v;
}
void Request::setUri(const std::string &v) {
    uri = v;
}
void Request::setProto(const std::string &v) {
    proto = v;
}
void Request::setBody(const std::string &v) {
    body = v;
}
void Request::setMessage(const std::string &v) {
    message = v;
}

void Request::setHeaders(const std::unordered_map<std::string, std::string> &h) {
    headers = h;
}

void Request::setSingleCookie(const std::string &name, const std::string &value) {
    cookies[name] = value;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Query-string parser
//  Stores each key=value pair from the URI into queryParams.
// ─────────────────────────────────────────────────────────────────────────────

void Request::setParams(const std::string &rawUri) {
    queryParams.clear();
    size_t q = rawUri.find('?');
    if (q == std::string::npos || q + 1 >= rawUri.size()) return;

    std::string qs = rawUri.substr(q + 1);
    std::istringstream ss(qs);
    std::string pair;

    while (std::getline(ss, pair, '&')) {
        if (pair.empty()) continue;
        size_t eq = pair.find('=');
        if (eq != std::string::npos) queryParams[pair.substr(0, eq)] = pair.substr(eq + 1);
        else
            queryParams[pair] = "";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Route-template param extractor
//  e.g. pattern "/users/:id", uri "/users/42" → routeTemplateParams[":id"] = "42"
// ─────────────────────────────────────────────────────────────────────────────

void Request::setRouteTemplateParams(const std::string &pattern, const std::string &rawUri) {
    routeTemplateParams.clear();
    auto pSegs = split_path(pattern);
    auto uSegs = split_path(rawUri);
    if (pSegs.size() != uSegs.size()) return;

    for (size_t i = 0; i < pSegs.size(); ++i) {
        if (pSegs[i].front() == ':') routeTemplateParams[pSegs[i]] = uSegs[i];
        else if (pSegs[i] != uSegs[i])
            return; // mismatch — clear and bail
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Cookie parser
// ─────────────────────────────────────────────────────────────────────────────

void Request::parseCookies(const std::unordered_map<std::string, std::string> &hdrs) {
    auto it = hdrs.find("Cookie");
    if (it == hdrs.end()) return;

    std::istringstream ss(it->second);
    std::string token;
    while (std::getline(ss, token, ';')) {
        size_t eq = token.find('=');
        if (eq == std::string::npos) continue;
        std::string name = token.substr(0, eq);
        std::string value = token.substr(eq + 1);
        trim(name);
        trim(value);
        setSingleCookie(name, value);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  buildRequest
//
//  Parses the raw HTTP message into structured fields.
//
//  Body handling change (RFC 2616 §4.4):
//    readSocket() now reads the full body before this function is called,
//    so the text after "\r\n\r\n" is the complete body — no extra reading
//    needed here.  Previously the body was silently discarded.
// ─────────────────────────────────────────────────────────────────────────────

void Request::buildRequest(std::string &msg, Router &router) {
    setMessage(msg);

    // ── Extract body (everything after the blank line) ────────────────────
    size_t bodyStart = msg.find("\r\n\r\n");
    setBody(bodyStart != std::string::npos ? msg.substr(bodyStart + 4) : "");

    // ── Parse the request line ────────────────────────────────────────────
    size_t methodEnd = msg.find(' ');
    size_t uriEnd = msg.find(' ', methodEnd + 1);
    size_t protoEnd = msg.find("\r\n", uriEnd + 1);

    if (methodEnd == std::string::npos || uriEnd == std::string::npos || protoEnd == std::string::npos) {
        // Malformed request line — surface as a 400 in the caller
        setMethod("INVALID");
        setUri("/");
        setProto("HTTP/1.1");
        return;
    }

    std::string rawMethod = msg.substr(0, methodEnd);
    std::string rawUri = msg.substr(methodEnd + 1, uriEnd - methodEnd - 1);
    std::string rawProto = msg.substr(uriEnd + 1, protoEnd - uriEnd - 1);

    setMethod(rawMethod);
    setProto(rawProto);
    setHeaders(extractHttpHeader(msg));
    setParams(rawUri);
    parseCookies(headers);
    setUri(extractMainRoute(rawUri)); // strip query string for routing

    // Route-template params need the path-only URI (no query string)
    std::string tmpl = router.findMatchingRouteTemplate(uri);
    setRouteTemplateParams(tmpl, uri);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

std::unordered_map<std::string, std::string> Request::extractHttpHeader(const std::string &msg) {
    std::unordered_map<std::string, std::string> hdrs;
    std::istringstream ss(msg);
    std::string line;

    std::getline(ss, line); // skip request line

    while (std::getline(ss, line) && !line.empty() && line != "\r") {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string name = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        trim(name);
        trim(value);
        hdrs[name] = value;
    }
    return hdrs;
}

std::string Request::extractMainRoute(const std::string &url) {
    size_t q = url.find('?');
    return q != std::string::npos ? url.substr(0, q) : url;
}

std::string Request::contentLength() const {
    return std::to_string(body.size());
}

std::string Request::getHeaderValue(const std::string &name) const {
    auto it = headers.find(name);
    return it != headers.end() ? it->second : "";
}

std::vector<std::string> Request::split_path(const std::string &path, char delim) {
    std::vector<std::string> segs;
    std::stringstream ss(path);
    std::string seg;
    while (std::getline(ss, seg, delim))
        if (!seg.empty()) segs.push_back(seg);
    return segs;
}

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