/**
 * http_tcpServer_linux.h
 * http_tcpServer_linux.cpp -- will hold the code for our actual server
 * implementation via the TcpServer class.
 *
 *
 * server_linux.cpp -- will have a “main” function through which
 * will run the server using the TcpServer object.
 */

#include <signal.h>

#include <future>
#include <iostream>
#include <typeinfo>

#include "cpp_webserver/http_server_linux.hpp"

std::atomic_bool global_run_flag(true);
using namespace http;
TcpServer server = TcpServer("0.0.0.0", 8080);

void signal_callback_handler(int signum) {
    std::cout << "Caught signal " << signum << std::endl;
    // Terminate program
    global_run_flag = false;
    server.closeServer();
}

int main() {
    signal(SIGINT, &signal_callback_handler);
    server.startListen(&global_run_flag);

    // std::cout << "global_run_flag" << std::boolalpha << global_run_flag << std::endl;
    // server.closeServer();
    return 0;
}