#include <string>
#include "cpp_webserver_include/core.hpp"

// TODO:
// - bug with setting headers on request from main.cpp

int main()
{
    HttpServer server("127.0.0.1", "8080");

    // add additional routes
    server.addRoute("/Custom2", [](Request &httpRequest, Response &httpResponse)
                    {

        httpRequest.setHeaders({
            {"Content-Type", "text/html"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });
        // set headers
        httpResponse.setHeaders({
            {"Content-Type", "text/html"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });

        // response
        std::string response = "<html><body>Hello!</body></html>";
        httpResponse.GET(response); });

    server.addRoute("/testing", [](Request &httpRequest, Response &httpResponse)
                    {
        // set headers
        httpResponse.setHeaders({
            {"Content-Type", "application/json"},
            {"Connection", "keep-alive"},
            {"Accept-Encoding", "gzip, deflate, br",}
        });

        std::string paramValue = httpRequest.returnParamValue("testing2");

        // response
        std::string response ="{\"message\": \"" + paramValue + "\"}";
        httpResponse.GET(response); });

    server.addRoute("/other", [](Request &httpRequest, Response &httpResponse)
                    {
                        // set headers
                        httpResponse.setHeaders({{"Content-Type", "text/html"},
                                                 {"Connection", "keep-alive"},
                                                 {
                                                     "Accept-Encoding",
                                                     "gzip, deflate, br",
                                                 }});

                        // response
                        std::string response = "<html><body>Page 3!</body></html>";
                        httpResponse.PUT(response); });

    server.printRoutes();
    server.start();

    return 0;
}