/**
 * http_tcpServer_linux.h
 * http_tcpServer_linux.cpp -- will hold the code for our actual server
 * implementation via the TcpServer class.
 *
 *
 * server_linux.cpp -- will have a “main” function through which
 * will run the server using the TcpServer object.
 */

#include <iostream>

#include "cpp_webserver/http_tcpServer_linux.hpp"

int main() {
    using namespace http;
    TcpServer server = TcpServer("0.0.0.0", 8080);
    bool serverClosed = server.startListen();

    // TODO: doesn't work. Server needs to be closed properly
    // std::cout << "Server status: " << serverClosed << std::endl;
    // if (serverClosed) {
    //     server.closeServer();
    // }

    return 0;
}