#include <iostream>
#include <string>

namespace logger {
    const int BUFFER_SIZE = 30720;

    void log(const std::string &message, const char* functionName = __builtin_FUNCTION());
    void exitWithError(const std::string &errorMessage, const char* functionName = __builtin_FUNCTION());

}  // namespace