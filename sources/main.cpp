#include <Server.hpp>

#include "./config/config.hpp"
#define PORT "3490"

// TESTING: run "nc localhost 3490" in another
// terminal (or multiple terminals to simulate multiple clients)
// and start typing messages
int main() {
  Server server(PORT);
  server.runServer();
}

// int main(int argc, char **argv) {
//     if (argc != 2)
//     {
//         std::cerr << "Error: Invalid number of arguments" << std::endl;
//         return 1;
//     }
//     Config config(argv[1]);
//     config.printServerConfig();
//     return 0;
//     return 0;
// }
