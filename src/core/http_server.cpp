#include "cpp_webserver_include/core.hpp"

#include <arpa/inet.h>
#include <netdb.h> // NEW
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

// ── Construction / Destruction ───────────────────────────────────────────────

HttpServer::HttpServer(const char *ip, const char *port) : ip_address(ip), port(port), server_socket(0) {}

HttpServer::~HttpServer() {
    close(server_socket);
}

// ── Entry Point ──────────────────────────────────────────────────────────────

void HttpServer::start(std::function<void()> onReady) {
    if (setupServerSocket() && listenSocket()) {
        if (onReady) onReady();
        acceptConnections();
    }
    close(server_socket);
}

// -- Log TCP Diagnostics available on Linux
void logTcpDiagnostics(int clientFd) {
    struct tcp_info ti{};
    socklen_t tiLen = sizeof(ti);

    if (getsockopt(clientFd, IPPROTO_TCP, TCP_INFO, &ti, &tiLen) != 0) {
        std::perror("getsockopt(TCP_INFO) failed");
        return;
    }

    char src[INET6_ADDRSTRLEN]{};
    char dst[INET6_ADDRSTRLEN]{};

    struct sockaddr_storage addr{};
    socklen_t len = sizeof(addr);

    // Get peer (remote) address
    if (getpeername(clientFd, (struct sockaddr *)&addr, &len) == 0) {
        void *srcPtr = (addr.ss_family == AF_INET) ? (void *)&((struct sockaddr_in *)&addr)->sin_addr
                                                   : (void *)&((struct sockaddr_in6 *)&addr)->sin6_addr;

        inet_ntop(addr.ss_family, srcPtr, src, sizeof(src));
    } else {
        std::strcpy(src, "unknown");
    }

    // Get local address
    len = sizeof(addr);
    if (getsockname(clientFd, (struct sockaddr *)&addr, &len) == 0) {
        void *dstPtr = (addr.ss_family == AF_INET) ? (void *)&((struct sockaddr_in *)&addr)->sin_addr
                                                   : (void *)&((struct sockaddr_in6 *)&addr)->sin6_addr;

        inet_ntop(addr.ss_family, dstPtr, dst, sizeof(dst));
    } else {
        std::strcpy(dst, "unknown");
    }

    std::cout << "[Connection] " << src << " -> " << dst << " | Retransmits: " << static_cast<int>(ti.tcpi_retransmits)
              << " | CWND: " << static_cast<int>(ti.tcpi_snd_cwnd) << '\n';
}

// -- Middleware --
void HttpServer::runMiddlewareChain(Request &req, Response &res) {
    size_t index = 0;

    std::function<void()> next = [&]() {
        if (index < middlewares.size()) {
            auto &mw = middlewares[index++];
            mw(req, res, next);
        } else {
            // FINAL step → route handling
            if (!router_.dispatch(req, res)) {
                res.setStatus(404).setBody("Not Found").setHeader("Content-Type", "text/plain");
            }
        }
    };

    next();
}

// ── Accept Loop ──────────────────────────────────────────────────────────────

void HttpServer::acceptConnections() {
    sockaddr_storage clientAddr;

    while (true) {
        socklen_t addrLen = sizeof(clientAddr); // reset each time
        int fd = accept(server_socket, (sockaddr *)&clientAddr, &addrLen);
        if (fd == -1) {
            perror("accept");
            break;
        }

        logTcpDiagnostics(fd);

        if (activeConnections.load() >= MAX_CONNECTIONS) {
            static constexpr char busy[] =
                "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            send(fd, busy, sizeof(busy) - 1, 0);
            close(fd);
            continue;
        }

        ++activeConnections;
        std::thread([this, fd] {
            handleConnection(fd);
            --activeConnections;
        }).detach();
    }
}

// ── Socket Lifecycle ─────────────────────────────────────────────────────────

bool HttpServer::setupServerSocket() {
    struct addrinfo hints{};
    struct addrinfo *servinfo;
    struct addrinfo *p;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 OR IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // bind to local IP

    int status = getaddrinfo(ip_address, port, &hints, &servinfo);
    if (status != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << "\n";
        return false;
    }

    // Loop through all results and bind to first valid one
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_socket == -1) continue;

        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(server_socket, p->ai_addr, p->ai_addrlen) == 0) {
            break; // SUCCESS
        }

        close(server_socket);
    }

    freeaddrinfo(servinfo);

    if (p == nullptr) {
        std::cerr << "Failed to bind to any address\n";
        return false;
    }

    return true;
}

// bool HttpServer::createSocket() {
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket == -1) {
//         perror("socket");
//         return false;
//     }
//     return true;
// }

// bool HttpServer::bindSocket() {
//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = inet_addr(ip_address);
//     addr.sin_port = htons(std::stoi(port));

//     int opt = 1;
//     if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
//         bind(server_socket, (sockaddr *)&addr, sizeof(addr)) == -1) {
//         perror("bind");
//         return false;
//     }
//     return true;
// }

bool HttpServer::listenSocket() {
    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("listen");
        return false;
    }
    return true;
}

// ── Read One HTTP Request ────────────────────────────────────────────────────

static std::string readSocket(int fd) {
    std::string data;
    char buf[4096];

    while (data.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            if (n < 0) throw std::runtime_error("recv failed");
            break;
        }
        data.append(buf, n);
    }
    return data;
}

// ── Request / Response ───────────────────────────────────────────────────────

void HttpServer::handleRequest(const std::string &raw, Request &req, Response &res) {
    auto t0 = std::chrono::high_resolution_clock::now();
    // std::cout << "Request:\n" << raw << "\n";
    req = Request::parse(raw);
    res.setHeader("Connection", req.headers.count("Connection") ? req.headers.at("Connection") : "keep-alive");
    runMiddlewareChain(req, res);
}

void HttpServer::handleResponse(int fd, Response &res) {
    std::string out = res.build();
    if (send(fd, out.c_str(), out.size(), 0) < 0) std::cerr << "send error\n";
}

// ── Per-Connection Handler ───────────────────────────────────────────────────

void HttpServer::handleConnection(int fd) {
    timeval tv{KEEPALIVE_TIMEOUT_SEC, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (true) {

        std::string raw;
        try {
            raw = readSocket(fd);
        } catch (...) { break; }
        if (raw.empty()) break;

        // // ── Upgrade to WebSocket if requested
        // if (raw.find("Upgrade: websocket") != std::string::npos) {
        //     if (ws_) ws_->handleUpgrade(fd, raw); // hand off entirely
        //     break;                                // HTTP is done on this fd
        // }

        // ── Normal HTTP
        Request req;
        Response res;

        handleRequest(raw, req, res);
        handleResponse(fd, res);
    }
    close(fd);
}