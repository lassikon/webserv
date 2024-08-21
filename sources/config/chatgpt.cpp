#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <utility>

// Structures to hold server and route configurations
struct RouteConfig {
    std::string path;
    std::string root;
    std::vector<std::string> methods;
};

struct ServerConfig {
    std::string host;
    int port;
    std::string server_name;
    std::map<std::string, RouteConfig> routes;  // Maps route paths to RouteConfig
};

// Function to trim whitespace
std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

int main() {
    std::ifstream config_file("config.ini");
    if (!config_file) {
        std::cerr << "Unable to open configuration file." << std::endl;
        return 1;
    }

    // Read the entire file into a stringstream
    std::stringstream buffer;
    buffer << config_file.rdbuf();
    config_file.close();

    std::string line;
    std::map<std::string, ServerConfig> servers;  // Maps server names to ServerConfig
    ServerConfig current_server;
    RouteConfig current_route;
    std::string current_section;
    std::string current_server_name;

    // Parse the configuration
    while (std::getline(buffer, line)) {
        // Remove comments and trim whitespace
        line = line.substr(0, line.find('#'));
        line = trim(line);

        if (line.empty()) continue;  // Skip empty lines

        if (line.front() == '[' && line.back() == ']') {
            // Section header
            current_section = line.substr(1, line.size() - 2);

            if (current_section == "server") {
                current_server = ServerConfig();  // Reset server configuration
            } else if (current_section == "route") {
                current_route = RouteConfig();  // Reset route configuration
            }

            // If it's a server section, store the server name
            if (current_section == "server" && !current_server.server_name.empty()) {
                servers[current_server.server_name] = current_server;
            }

        } else if (!current_section.empty()) {
            // Key-value pairs
            auto delimiter_pos = line.find('=');
            std::string key = trim(line.substr(0, delimiter_pos));
            std::string value = trim(line.substr(delimiter_pos + 1));

            if (current_section == "server") {
                if (key == "host") current_server.host = value;
                else if (key == "port") current_server.port = std::stoi(value);
                else if (key == "server_name") {
                    current_server.server_name = value;
                    current_server_name = value;  // Set the current server name
                }
            } else if (current_section == "route") {
                if (key == "path") current_route.path = value;
                else if (key == "root") current_route.root = value;
                else if (key == "methods") {
                    std::stringstream ss(value);
                    std::string method;
                    while (std::getline(ss, method, ',')) {
                        current_route.methods.push_back(trim(method));
                    }
                }

                // Add the route to the current server's route map
                if (!current_server_name.empty() && !current_route.path.empty()) {
                    servers[current_server_name].routes[current_route.path] = current_route;
                }
            }
        }
    }

    // Output the parsed configuration
    for (const auto& server_pair : servers) {
        const auto& server = server_pair.second;
        std::cout << "Server host: " << server.host << std::endl;
        std::cout << "Server port: " << server.port << std::endl;
        std::cout << "Server name: " << server.server_name << std::endl;

        for (const auto& route_pair : server.routes) {
            const auto& route = route_pair.second;
            std::cout << "  Route path: " << route.path << std::endl;
            std::cout << "  Route root: " << route.root << std::endl;
            std::cout << "  Route methods: ";
            for (const auto& method : route.methods) {
                std::cout << method << " ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
