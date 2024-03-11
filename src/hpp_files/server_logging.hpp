#pragma once

#include <iostream>
#include <string>

class MyCustomException : public std::exception {
   private:
    const char* message;

   public:
    MyCustomException(const char* msg) : message(msg) {}
    const char* what() {
        return message;
    }
};

namespace logger {
const int BUFFER_SIZE = 30720;

void log(const std::string& message, const char* functionName = __builtin_FUNCTION());
void exitWithError(const std::string& errorMessage, const char* functionName = __builtin_FUNCTION());

}  // namespace logger