#include <Server.hpp>

#define PORT "3490"

// TESTING: run "nc localhost 3490" in another terminal and start typing messages
int main() {
  Server server(PORT);
  server.run();
}
// #include <Exception.hpp>

// class Test {
// public:
//   void callChecker(int argc) { privateFunc(argc); };
//   void checkArgCount(int argc) {
//     if (argc > 1)
//       THROW_WARN(ERR_MSG_USAGE);
//   }

// private:
//   void privateFunc(int argc) {
//     Exception::tryCatch(&Test::checkArgCount, this, argc);
//   }
// };

// int main(int argc, char **argv) {
//   (void)argv;
//   Test t;
//   t.callChecker(argc);
//   Exception::tryCatch(&Test::checkArgCount, &t, argc);
//   return 0;
// }

// int main()
// {
//     struct addrinfo hints;
//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     std::cout << "getaddrinfo()" << std::endl;
//     struct addrinfo *res;
//     getaddrinfo(NULL, PORT, &hints, &res);

//     std::cout << "socket()" << std::endl;
//     int serverFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

//     std::cout << "bind()" << std::endl;
//     bind(serverFD, res->ai_addr, res->ai_addrlen);

//     std::cout << "listen()" << std::endl;
//     listen(serverFD, 10); // Backlog 10

//     struct pollfd fds[MAX_CLIENTS];
//     memset(fds, 0, sizeof(fds));
//     fds[0].fd = serverFD; // Listening socket
//     fds[0].events = POLLIN;

//     int nfds = 1;
//     while (true)
//     {
//         // std::cout << "poll()" << std::endl;
//         int pollCount = poll(fds, nfds, -1);
//         if (pollCount == -1)
//         {
//             std::cout << "poll error" << std::endl;
//             exit(EXIT_FAILURE);
//         }

//         // Check the listening socket for new connections
//         if (fds[0].revents & POLLIN)
//         {
//             struct sockaddr_storage their_addr;
//             socklen_t addr_size = sizeof their_addr;
//             int clientFD = accept(serverFD, (struct sockaddr *)&their_addr, &addr_size);
//             if (clientFD == -1)
//             {
//                 std::cout << "accept error" << std::endl;
//                 exit(EXIT_FAILURE);
//             }
//             else
//             {
//                 // Add the new client socket to the poll list
//                 LOG_INFO("accepted client fd: ", clientFD);
//                 fds[nfds].fd = clientFD;
//                 fds[nfds].events = POLLIN;
//                 nfds++;
//             }
//         }

//         // Check existing connections for incoming data
//         char buf[1024];
//         bool allConnectionsClosed = false;
//         for (int i = 1; i < nfds; i++)
//         {
//             memset(buf, 0, sizeof(buf));
//             if (fds[i].revents & POLLIN)
//             {
//                 int receivedBytes = recv(fds[i].fd, buf, sizeof buf, 0);
//                 if (receivedBytes < 0)
//                 {
//                     std::cout << "recv error" << std::endl;
//                     exit(EXIT_FAILURE);
//                 }
//                 else if (receivedBytes == 0)
//                 {
//                     // Connection closed
//                      close(fds[i].fd);
//                      fds[i] = fds[nfds-1];
//                     nfds--;
//                     if (nfds == 1)
//                         allConnectionsClosed = true;
//                     LOG_INFO("connection closed for client ", fds[i].fd);
//                     i--;
//                 }
//                 else
//                 {
//                     // Process the received data
//                     LOG_INFO("receiving data from client ", fds[i].fd, ", buffer: ", buf);
//                     int sentBytes = send(fds[i].fd, buf, receivedBytes, 0);

//                     if (sentBytes == -1)
//                     {
//                         std::cout << "send error" << std::endl;
//                         close(fds[i].fd);
//                         fds[i] = fds[nfds-1];
//                         nfds--;
//                         i--;
//                     }
//                 }
//             }
//         }
//         if (allConnectionsClosed == true)
//         {
//             std::cout << "all connections closed" << std::endl;
//             break ;
//         }
//     }

//     std::cout << "freeaddrinfo()" << std::endl;
//     freeaddrinfo(res);
//     std::cout << "close(serverFD)" << std::endl;
//     close(serverFD);

//     return EXIT_SUCCESS;
// }

// int main() {
//   LOG_TRACE("Hello ", 123);
//   LOG_DEBUG("Hello ", 123);
//   LOG_INFO("Hello ", 123);
//   LOG_WARNING("Hello ", 123);
//   LOG_ERROR("Hello ", 123);
//   LOG_CRITICAL("Hello ", 123);
//   return 0;
// }
