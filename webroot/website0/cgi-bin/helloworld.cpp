#include <iostream>

int main() {
  std::cout << "Connection: keep-alive\n";
  std::cout << "Content-type: text/html\n\n";
  std::cout << "<html>\n";
  std::cout << "<head>\n";
  std::cout << "<title>Hello World CPP</title>\n";
  std::cout << "</head>\n";
  std::cout << "<body>\n";
  std::cout << "<h1>Hello World! This is header one</h1>\n";
  std::cout << "</body>\n";
  std::cout << "</html>\n";
  return 0;
}
