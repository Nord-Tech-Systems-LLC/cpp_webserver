#include "cpp_webserver_include/core.hpp"

#ifdef __linux__
// linux libraries
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // for TCP_INFO
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
// windows libraries
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <algorithm>
#include <chrono> // For timing
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

// ─────────────────────────────────────────────────────────────────────────────
//  Utilities
// ─────────────────────────────────────────────────────────────────────────────

// RFC 2616 §14.18 — RFC 1123 GMT date string required on every response.
static std::string httpDate() {

    // Format is RFC 1123: "Thu, 01 Jan 1970 00:00:00 GMT"
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
    return buf;
}

// Combined Log Format: "METHOD URI PROTO" STATUS BYTES "Referer" "UA" "time"
static std::string toSummaryFormat(const std::string &method, const std::string &uri, const std::string &proto,
                                   int status, size_t bytes, const std::string &referer, const std::string &ua,
                                   const std::string &elapsed) {
    std::ostringstream o;
    o << '"' << method << ' ' << uri << ' ' << proto << "\" " << status << ' ' << bytes << " \"" << referer << "\" \""
      << ua << "\" \"" << elapsed << '"';
    return o.str();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Low-level socket I/O  (free functions — no shared HttpServer state)
// ─────────────────────────────────────────────────────────────────────────────

static void closeSocket(int fd) {
#ifdef __linux__
    close(fd);
#elif _WIN32
    closesocket(fd);
#endif
}

/**
 * Read one complete HTTP request from the socket.
 *
 * RFC 2616 §4.4 — message-body length rules:
 *   Phase 1: Read until the blank line ("\r\n\r\n") that ends the headers.
 *   Phase 2: If Content-Length is present, read exactly that many body bytes.
 */
static std::string readSocket(int fd) {
    std::string data;
    char buf[4096];

    // ── Phase 1: headers ─────────────────────────────────────────────────
    while (true) {
        memset(buf, 0, sizeof(buf));
#ifdef __linux__
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
#elif _WIN32
        SSIZE_T n = recv(fd, buf, sizeof(buf) - 1, 0);
#endif
        if (n < 0) throw std::runtime_error("Error reading from socket");
        if (n == 0) break;
        data.append(buf, n);
        if (data.find("\r\n\r\n") != std::string::npos) break;
    }

    size_t headerEnd = data.find("\r\n\r\n");
    if (headerEnd == std::string::npos) return data;

    std::string headerSection = data.substr(0, headerEnd);
    std::string bodyAlreadyRead = data.substr(headerEnd + 4);

    // ── Phase 2: body (Content-Length, case-insensitive search) ──────────
    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    };
    std::string lowerHeaders = toLower(headerSection);
    size_t clPos = lowerHeaders.find("content-length:");
    if (clPos != std::string::npos) {
        size_t vs = headerSection.find(':', clPos) + 1;
        size_t ve = headerSection.find("\r\n", vs);
        std::string clVal = headerSection.substr(vs, ve == std::string::npos ? ve : ve - vs);
        trim(clVal);

        size_t contentLength = 0;
        try {
            contentLength = std::stoul(clVal);
        } catch (...) {}

        while (bodyAlreadyRead.size() < contentLength) {
            size_t needed = contentLength - bodyAlreadyRead.size();
            size_t toRead = std::min(needed, sizeof(buf) - 1);
            memset(buf, 0, sizeof(buf));
#ifdef __linux__
            ssize_t n = recv(fd, buf, toRead, 0);
#elif _WIN32
            SSIZE_T n = recv(fd, buf, toRead, 0);
#endif
            if (n <= 0) break;
            bodyAlreadyRead.append(buf, n);
        }
        data = headerSection + "\r\n\r\n" + bodyAlreadyRead;
    }

    return data;
}

// Sends all bytes; does NOT close fd (keep-alive requires caller to own lifetime).
static void sendSocket(int fd, const std::string &data) {
    // required for RFC 2616 §8.1 persistent connections).
    size_t sent = 0;
    while (sent < data.size()) {
#ifdef __linux__
        ssize_t n = send(fd, data.c_str() + sent, data.size() - sent, 0);
#elif _WIN32
        SSIZE_T n = send(fd, data.c_str() + sent, data.size() - sent, 0);
#endif
        if (n < 0) throw std::runtime_error("Error sending data to socket");
        sent += static_cast<size_t>(n);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  HttpServer — construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

HttpServer::HttpServer(const char *ip_address, const char *port)
    : ip_address(ip_address), port(port), server_socket(0) {}

HttpServer::~HttpServer() {
// close the server socket
#ifdef __linux__
    close(server_socket);
#elif _WIN32
    closesocket(server_socket);
    // cleanup Winsock
    WSACleanup();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Socket lifecycle
// ─────────────────────────────────────────────────────────────────────────────

bool HttpServer::createSocket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return false;
    }
    return true;
}

bool HttpServer::bindSocket() {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_address);
    addr.sin_port = htons(std::stoi(port));

#ifdef __linux__
    int opt = 1;
#elif _WIN32
    const char opt = 1;
#endif
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Setsockopt failed");
        return false;
    }
    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Bind failed");
        return false;
    }
    return true;
}

bool HttpServer::listenSocket() {
    // Use MAX_CONNECTIONS as the backlog so the kernel can queue up to that many
    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("Listen failed");
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  IP helpers
// ─────────────────────────────────────────────────────────────────────────────

// gets the server IP
std::string HttpServer::getServerIP(int client_socket) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (getsockname(client_socket, (struct sockaddr *)&addr, &len) == -1) {
        perror("getsockname failed");
        return "";
    }

    char ipstr[INET6_ADDRSTRLEN];

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
        return std::string(ipstr) + ":" + std::to_string(ntohs(s->sin_port));
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
        return std::string(ipstr) + ":" + std::to_string(ntohs(s->sin6_port));
    }

    return "Unknown";
}

// gets the client IP
std::string HttpServer::getClientIP(int client_socket) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (getpeername(client_socket, (struct sockaddr *)&addr, &len) == -1) {
        perror("getpeername failed");
        return "";
    }

    char ipstr[INET6_ADDRSTRLEN];

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
        return std::string(ipstr) + ":" + std::to_string(ntohs(s->sin_port));
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
        return std::string(ipstr) + ":" + std::to_string(ntohs(s->sin6_port));
    }

    return "Unknown";
}

// helper function to convert a struct sockaddr address to a string, IPv4 and IPv6
char *HttpServer::get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
    switch (sa->sa_family) {
        // IPv4
    case AF_INET:
        inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
        break;

        // IPv6
    case AF_INET6:
        inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
        break;

        // Error
    default:
        strncpy(s, "Unknown AF", maxlen);
        return NULL;
    }

    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Server entry point
// ─────────────────────────────────────────────────────────────────────────────

void HttpServer::start() {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa;
    int init_winsock = WSAStartup(MAKEWORD(2, 0), &wsa);
    if (init_winsock != 0) { printf("WSAStartup failed: %d\n", init_winsock); }
#endif

    if (createSocket() && bindSocket() && listenSocket()) {
        std::cout << "Server listening on port " << port << std::endl;
        acceptConnections();
    }

// close the server socket
#ifdef __linux__
    close(server_socket);
#elif _WIN32
    closesocket(server_socket);
    // cleanup Winsock
    WSACleanup();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Accept loop
//
//  RFC 2616 §8.1.4 — servers SHOULD limit simultaneous connections.
//  If MAX_CONNECTIONS is reached the new socket gets an immediate 503 and
//  is closed, so the client gets a proper error rather than hanging.
// ─────────────────────────────────────────────────────────────────────────────

void HttpServer::acceptConnections() {
    struct sockaddr_storage clientAddr;
#ifdef __linux__
    socklen_t addrLen = sizeof(clientAddr);
#elif _WIN32
    int addrLen = sizeof(clientAddr);
#endif

    while (true) {
        int clientFd = accept(server_socket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientFd == -1) {
            perror("Accept failed");
            break;
        }

        std::string srcIP = getClientIP(clientFd);
        std::string dstIP = getServerIP(clientFd);

#ifdef __linux__
        // Log TCP diagnostics available on Linux
        struct tcp_info ti{};
        socklen_t tiLen = sizeof(ti);
        if (getsockopt(clientFd, IPPROTO_TCP, TCP_INFO, &ti, &tiLen) == 0) {
            char src[INET6_ADDRSTRLEN]{}, dst[INET6_ADDRSTRLEN]{};
            struct sockaddr_storage a{};
            socklen_t l = sizeof(a);
            getpeername(clientFd, (struct sockaddr *)&a, &l);
            inet_ntop(a.ss_family,
                      a.ss_family == AF_INET ? (void *)&((sockaddr_in *)&a)->sin_addr
                                             : (void *)&((sockaddr_in6 *)&a)->sin6_addr,
                      src, sizeof(src));
            getsockname(clientFd, (struct sockaddr *)&a, &l);
            inet_ntop(a.ss_family,
                      a.ss_family == AF_INET ? (void *)&((sockaddr_in *)&a)->sin_addr
                                             : (void *)&((sockaddr_in6 *)&a)->sin6_addr,
                      dst, sizeof(dst));
            std::cout << "[Connection] " << src << " -> " << dst << " | Retransmits: " << (int)ti.tcpi_retransmits
                      << " | CWND: " << (int)ti.tcpi_snd_cwnd << '\n';
        }
#else
        std::cout << "[Connection] " << srcIP << " -> " << dstIP << " | TCP info unavailable on this platform\n";
#endif

        // Enforce connection cap — return 503 immediately and move on
        if (activeConnections.load() >= MAX_CONNECTIONS) {
            const std::string busy = "HTTP/1.1 503 Service Unavailable\r\n"
                                     "Content-Length: 0\r\nConnection: close\r\n\r\n";
            try {
                sendSocket(clientFd, busy);
            } catch (...) {}
            closeSocket(clientFd);
            continue;
        }

        activeConnections++;
        // Each connection gets its own thread. Request & Response objects are
        // created inside handleConnection — no shared mutable state.
        std::thread([this, clientFd]() {
            handleConnection(clientFd);
            activeConnections--;
        }).detach();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Per-connection handler — implements HTTP/1.1 persistent connections
//
//  RFC 2616 §8.1 — connections are persistent by default in HTTP/1.1.
//  The loop continues serving requests on the same socket until:
//    • client or server sends  Connection: close
//    • the keep-alive idle timeout fires  (SO_RCVTIMEO)
//    • a read / write error occurs
// ─────────────────────────────────────────────────────────────────────────────

void HttpServer::handleConnection(int fd) {
    // Idle keep-alive timeout — recv returns EAGAIN when it fires
    struct timeval tv{KEEPALIVE_TIMEOUT_SEC, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    bool keepAlive = true;
    while (keepAlive) {
        Request req;
        Response res;
        res.setSingleHeader("Date", httpDate());
        res.setSingleHeader("Server", "CppWebServer/1.0");

        std::string raw;
        try {
            raw = readSocket(fd);
        } catch (...) { break; }
        if (raw.empty()) break;

        handleRequest(raw, req, res);

        std::string conn = req.getHeaderValue("Connection");
        std::transform(conn.begin(), conn.end(), conn.begin(), ::tolower);
        keepAlive = (conn != "close");
        res.setSingleHeader("Connection", keepAlive ? "keep-alive" : "close");

        handleResponse(fd, res);
    }

    closeSocket(fd);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Single request / response cycle
// ─────────────────────────────────────────────────────────────────────────────

void HttpServer::handleRequest(const std::string &raw, Request &req, Response &res) {
    auto t0 = std::chrono::high_resolution_clock::now();

    req.buildRequest(const_cast<std::string &>(raw), router);
    res.setRequestMethod(req.method);

    // RFC 2616 §14.23 — reject HTTP/1.1 without Host
    if (req.proto == "HTTP/1.1" && req.getHeaderValue("Host").empty()) {
        res.setHeaders({{"Content-Type", "text/plain"}, {"Connection", "close"}});
        res.status(400).send("Bad Request: missing Host header");
        return;
    }

    // RFC 2616 §9.4 — HEAD reuses the GET handler; body is stripped after
    const bool isHead = (req.method == "HEAD");
    if (isHead) req.setMethod("GET");

    if (req.method == "OPTIONS") {
        // RFC 2616 §9.2 — reflect actual allowed methods for this URI
        auto methods = router.allowedMethods(req.uri);
        std::string allow;
        for (size_t i = 0; i < methods.size(); ++i) {
            if (i) allow += ", ";
            allow += methods[i];
        }
        res.setSingleHeader("Allow", allow);
        res.setSingleHeader("Access-Control-Allow-Origin", "*");
        res.setSingleHeader("Access-Control-Allow-Methods", allow);
        res.setSingleHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status(204).send("");

    } else if (!router.handleRoute(req, res)) {
        auto methods = router.allowedMethods(req.uri);
        if (!methods.empty()) {
            // RFC 2616 §10.4.6 — 405 must include Allow header
            std::string allow;
            for (size_t i = 0; i < methods.size(); ++i) {
                if (i) allow += ", ";
                allow += methods[i];
            }
            res.setSingleHeader("Allow", allow);
            res.setHeaders({{"Content-Type", "text/plain"}});
            res.status(405).send("Method Not Allowed");
        } else {
            logger::error("Route not found: " + req.uri);
            res.setHeaders({{"Content-Type", "text/plain"}});
            res.status(404).send("Not Found");
        }
    }

    if (isHead) {
        res.setBody("");
        req.setMethod("HEAD");
    }

    double elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count();
    logger::log(toSummaryFormat(req.method, req.uri, req.proto, res.getStatusCode(), res.getBody().size(),
                                req.getHeaderValue("Referer"), req.getHeaderValue("User-Agent"),
                                std::to_string(elapsed) + "s"));
}

void HttpServer::handleResponse(int clientFd, Response &res) {
    try {
        sendSocket(clientFd, res.getBody());
    } catch (const std::exception &e) { logger::error(std::string("handleResponse send error: ") + e.what()); }
    // Do NOT close the socket here. handleConnection owns the socket lifetime
    // so subsequent keep-alive requests can reuse it.
}

void HttpServer::printRoutes() {
    router.printRoutes();
}

// Add new middleware method that takes a map of headers
void HttpServer::middleware(const std::map<std::string, std::string> &headers) {
    router.use([headers](Request &req, Response &res) {
        for (const auto &header : headers) { res.setSingleHeader(header.first, header.second); }
    });
}

void HttpServer::get(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.get(route, handler);
}
void HttpServer::post(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.post(route, handler);
}
void HttpServer::put(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.put(route, handler);
}

void HttpServer::del(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.del(route, handler);
}

void HttpServer::use(std::function<void(Request &, Response &)> handler) {
    router.use(handler);
}

void HttpServer::use(const std::string &route, std::function<void(Request &, Response &)> handler) {
    router.use(route, handler);
}
