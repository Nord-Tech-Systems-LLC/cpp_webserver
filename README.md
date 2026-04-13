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
        int offset = std::stoi(req.query["offset"]);
        int limit = std::stoi(req.query["limit"]);

        res.setStatus(200).setBody("User List Data");
    } catch (const std::invalid_argument &e) {
        res.setStatus(400).setBody("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) { res.setStatus(500).setBody("Internal Server Error: " + std::string(e.what())); }
}

void Users::createUser(Request &req, Response &res) {
    try {
        res.setStatus(200).setBody(req.body);
    } catch (const std::invalid_argument &e) {
        res.setStatus(400).setBody("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) { res.setStatus(500).setBody("internal Server Error: " + std::string(e.what())); }
}

void Users::getCurrentUser(Request &req, Response &res) {
    try {
        // res.setHeader("Content-Type", "application/json");
        res.setStatus(200).setBody(req.body);
    } catch (const std::invalid_argument &e) {
        res.setStatus(400).setBody("Bad Request: " + std::string(e.what()));
    } catch (const std::exception &e) { res.setStatus(500).setBody("Internal Server Error: " + std::string(e.what())); }
}

void user_controller(HttpServer &server) {
    server.get("/api/list_users", Users::getUserListData);
    server.post("/api/current_user", Users::getCurrentUser);
    server.post("/api/create_user", Users::createUser);
}

int main() {
    HttpServer server("127.0.0.1", "8080");
    server.use([](Request &req, Response &res, Next next) {
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.setHeader("Access-Control-Allow-Headers", "Content-Type");

        if (req.method == "OPTIONS") {
            res.setStatus(204);
            return;
        }
        next();
    });
    user_controller(server); // load route controller
    server.printRoutes();
    server.start(); // start server
    return 0;
}

```
