// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "hpp_files/request.hpp"
#include "hpp_files/response.hpp"
#include "hpp_files/http_server.hpp"
// #include "hpp_files/server_logging.hpp"

int main()
{
    HttpServer server("127.0.0.1", "8080");
    
    // add additional routes
    server.addRoute("/Custom2", [](Request &httpRequest, Response &httpResponse) {
        // set headers
        httpResponse.setHeaders({
            {"Content-Type", "text/html"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });

        // response
        std::string response = "<html><body>Hello!</body></html>";
        httpResponse.GET(response);
    });

    server.printRoutes();
    server.start();


    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
