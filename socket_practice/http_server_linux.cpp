
#include "http_server_linux.hpp"

#include <unistd.h>

#include <fstream>
#include <future>
#include <iostream>
#include <sstream>
#include <string>

#include "cpp_webserver/server_logging.hpp"

namespace http {

TcpServer::TcpServer(std::string ip_address, int port)
    : _local_run_flag(true),
      m_ip_address(ip_address),
      m_port(port),
      m_socket(),
      m_new_socket(),
      m_incomingMessage(),
      m_socketAddress(),
      m_socketAddress_len(sizeof(m_socketAddress)),
      m_serverMessage(buildResponse()) {
    m_socketAddress.sin_family = AF_INET;
    m_socketAddress.sin_port = htons(m_port);
    m_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

    /**
     * debugging information
     */
    logger::log("m_ip_address: " + m_ip_address);
    logger::log("m_port: " + std::to_string(m_port));
    logger::log("m_socket: " + std::to_string(m_socket));
    logger::log("m_new_socket: " + std::to_string(m_new_socket));
    // logger::log("m_socketAddress: " + m_socketAddress);
    logger::log("m_socketAddress_len: " + std::to_string(m_socketAddress_len));
    logger::log("m_serverMessage: " + m_ip_address);

    if (startServer() != 0) {
        std::ostringstream ss;
        ss << "Failed to start server with PORT: " << ntohs(m_socketAddress.sin_port);
        logger::log(ss.str());
    }
}

// deconstructing TcpServer class
TcpServer::~TcpServer() {
    closeServer();
}

int TcpServer::startServer() {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        logger::exitWithError("Cannot create socket");
        return 1;
    }

    if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) < 0) {
        logger::exitWithError("Cannot connect socket to address");
        return 1;
    }

    return 0;
}

void TcpServer::closeServer() {
    logger::log("Shutting down server...");
    close(m_socket);
    close(m_new_socket);
    _local_run_flag = false;
    logger::log("Server closed");
    exit(0);
}

void TcpServer::startListen(std::atomic_bool global_run_flag) {
    if (listen(m_socket, 20) < 0) {
        logger::exitWithError("Socket listen failed");
    }

    std::string ip_address = inet_ntoa(m_socketAddress.sin_addr);
    int listening_port = ntohs(m_socketAddress.sin_port);
    std::ostringstream ss;
    ss << "Listening on PORT: " << listening_port << "; ADDRESS: http://" << ip_address << ":" << listening_port << "/\n";
    logger::log(ss.str());

    int bytesReceived;

    while (_local_run_flag && global_run_flag) {
        logger::log("====== Waiting for a new connection ======\n");
        acceptConnection(m_new_socket);

        char buffer[logger::BUFFER_SIZE] = {0};
        bytesReceived = read(m_new_socket, buffer, logger::BUFFER_SIZE);
        if (bytesReceived < 0) {
            logger::exitWithError("Failed to read bytes from client socket connection");
        }

        std::ostringstream ss;
        ss << "------ Received Request from client ------";
        logger::log(ss.str());

        sendResponse();

        close(m_new_socket);
    }
}

void TcpServer::acceptConnection(int &new_socket) {
    new_socket = accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_len);
    if (new_socket < 0) {
        std::ostringstream ss;
        ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << "; PORT: " << ntohs(m_socketAddress.sin_port);
        logger::exitWithError(ss.str());
    }
}

std::string TcpServer::buildResponse() {
    std::ostringstream ss;

    std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";

    ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
       << htmlFile;

    return ss.str();
}

void TcpServer::sendResponse() {
    long bytesSent;

    bytesSent = write(m_new_socket, m_serverMessage.c_str(), m_serverMessage.size());

    if (bytesSent == m_serverMessage.size()) {
        logger::log("------ Server Response sent to client ------");
    } else {
        logger::log("Error sending response to client");
    }
}

}  // namespace http