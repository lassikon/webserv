#include <Server.hpp>

#define PORT "3490"

// TESTING: run "nc localhost 3490" in another
// terminal (or multiple terminals to simulate multiple clients)
// and start typing messages
int main() {
  Server server(PORT);
  server.runServer();
}
