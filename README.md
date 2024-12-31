# C++ Backend Webserver

A modern C++ backend webserver library, using HTTP/1.

### to build project execute in root directory of project:

**Supported Operating Systems**: Linux, Windows

## Windows:

|       Requirement        | Value                                                 |
| :----------------------: | ----------------------------------------------------- |
| Minimum supported client | Windows 8.1, Windows Vista [desktop apps \| UWP apps] |
| Minimum supported server | Windows Server 2003 [desktop apps \| UWP apps]        |

Compiler: `Visual Studio Community 2022 Realease - amd64` - Using compilers for 17.9.5 (x64 Architecture)

## Linux:

```
$ ./build.sh
```

## Example usage:

```cpp
#include "cpp_webserver_include/core.hpp"
#include <iostream>

class Users {
  public:
    static void getUserListData(Request &req, Response &res);
    static void createUser(Request &req, Response &res);
    static void getCurrentUser(Request &req, Response &res);
};

void Users::getUserListData(Request &req, Response &res) {
    try {
        int offset = std::stoi(req.getRouteTemplateParams().at(":offset"));
        int limit = std::stoi(req.getRouteTemplateParams().at(":limit"));

        res.status(200).send("User List Data");
    } catch (const std::invalid_argument &e) {
        res.status(400).send("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) {
        res.status(500).send("Internal Server Error: " + std::string(e.what()));
    }
}

void Users::createUser(Request &req, Response &res) {
    try {
        res.status(200).send(req.getBody());
    } catch (const std::invalid_argument &e) {
        res.status(400).send("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) {
        res.status(500).send("internal Server Error: " + std::string(e.what()));
    }
}

void Users::getCurrentUser(Request &req, Response &res) {
    try {
        res.status(200).send("Current User!");
    } catch (const std::invalid_argument &e) {
        res.status(400).send("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) {
        res.status(500).send("Internal Server Error: " + std::string(e.what()));
    }
}

void user_controller(HttpServer &server) {
    server.get("/list_users/:offset/:limit", Users::getUserListData);
    server.get("/current_user", Users::getCurrentUser);
    server.post("/create_user", Users::createUser);
}

int main() {
    HttpServer server("127.0.0.1", "8080");
    server.middleware({{"Access-Control-Allow-Origin", "*"}}); // set global header config
    server.use(                                                // set middleware
        [](Request &req, Response &res) { std::cout << "This is middleware!" << std::endl; });
    user_controller(server); // load route controller
    server.start();          // start server
    return 0;
}

```
