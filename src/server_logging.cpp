#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace logger {
// colors for logging
const std::string red = "\033[1;31m";
const std::string green = "\033[1;32m";
const std::string yellow = "\033[1;33m";
const std::string blue = "\033[1;34m";
const std::string purple = "\033[1;35m";
const std::string cyan = "\033[1;36m";
const std::string grey = "\033[0;37m";
const std::string reset = "\033[m";

const int BUFFER_SIZE = 30720;

std::string returnCurrentTimeAndDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

void log(const std::string& message, const char* functionName = __builtin_FUNCTION()) {
    std::cout << green << "[LOG]" << reset << "[" << returnCurrentTimeAndDate() << "][" << functionName << "]: " << message << "\n";
}

void exitWithError(const std::string& errorMessage, const char* functionName = __builtin_FUNCTION()) {
    std::cout << red << "[ERROR]" << reset << "[" << returnCurrentTimeAndDate() << "][" << functionName << "]: " << errorMessage << "\n";
    // exit(1);
}
}  // namespace logger