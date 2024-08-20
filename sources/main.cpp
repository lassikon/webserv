#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    return EXIT_FAILURE;
  }

  sockaddr_in serverInfo;
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_port = htons(42);
  serverInfo.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (sockaddr *)&serverInfo, sizeof(serverInfo)) == -1) {
    return EXIT_FAILURE;
  };

  if (listen(serverSocket, SOMAXCONN) == -1) {
    return EXIT_FAILURE;
  };

  sockaddr_in client;
  int clientSocket = accept(serverSocket, (sockaddr *)&client, sizeof(client));
  if (clientSocket == -1) {
    return EXIT_FAILURE;
  }

  char buffer[4096];
  while (true) {
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived == -1) {
      return EXIT_FAILURE;
    } else if (!bytesReceived) {
      return EXIT_SUCCESS;
    } else {
      std::cout << buffer << std::endl;
      send(clientSocket, buffer, bytesReceived + 1, 0);
    }
  }

  close(serverSocket);
  std::cout << "Finished execution" << std::endl;
  return EXIT_SUCCESS;
}
