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

int main()
{
    HttpServer server("127.0.0.1", "8080");

    server.get("/", [](Request &req, Response &res)
               { res.send("Hello, world!"); });

    server.get(
        "/json", [&](Request &req, Response &res)
        { res.status(200).json("{\"message\": \"Hello, JSON\"}"); });

    server.post("/submit", [](Request &req, Response &res)
                {
        // Handle POST data
        res.status(201).send("Submitted!"); });
    server.printRoutes();
    server.start();

    return 0;
}

```
