
#ifndef GLOBAL_UTIL_H
#define GLOBAL_UTIL_H
#include <string>
#include <unordered_map>

/**
 * global variables needed for project
 */
extern const std::unordered_map<int, std::string> http_status_codes;
void trim(std::string &s);

#endif