#pragma once

class Server {
private:
  const float example = 42;

public:
  Server(void);
  Server(const Server &other);
  Server &operator=(const Server &other);
  ~Server(void);
};

/*
int getaddrinfo(const char *node,             // host name or IP to connect to
                const char *service,          // "http" or port number
                const struct addrinfo *hints, // addrinfo* that you’ve already filled out with relevant information.
                struct addrinfo **res);       // points to a linked list of addrinfos
*/

/*
struct addrinfo {
  int     ai_flags;          // AI_PASSIVE, AI_CANONNAME, ...
  int     ai_family;         // AF_xxx
  int     ai_socktype;       // SOCK_xxx
  int     ai_protocol;       // 0 (auto) or IPPROTO_TCP, IPPROTO_UDP 

  socklen_t  ai_addrlen;     // length of ai_addr
  char   *ai_canonname;      // canonical name for nodename
  struct sockaddr  *ai_addr; // binary address
  struct addrinfo  *ai_next; // next structure in linked list
};
*/