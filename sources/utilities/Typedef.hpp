#include <chrono>
#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string> ENVPMAP;
typedef std::unordered_map<int, std::string> SIGMAP;
typedef std::chrono::time_point<std::chrono::steady_clock> steady_time_point_t;
