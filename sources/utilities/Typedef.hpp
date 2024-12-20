#include <chrono>
#include <string>
#include <unordered_map>

typedef std::unordered_map<int, std::string> sigmap_t;
typedef std::chrono::time_point<std::chrono::steady_clock> steady_time_point_t;
typedef std::unordered_map<std::string, std::chrono::system_clock::time_point> cookiemap_t;
