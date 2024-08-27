#include <Exception.hpp>
#include <Server.hpp>

class Test {
public:
  void callChecker(int argc) { privateFunc(argc); };
  void checkArgCount(int argc) {
    if (argc)
      THROW_WARN(ERR_MSG_USAGE);
    std::cout << "This line would execute without throw!" << std::endl;
  }

private:
  void privateFunc(int argc) {
    Exception::tryCatch(&Test::checkArgCount, this, argc);
  }
};

int main(int argc, char **argv) {
  (void)argv;
  Test t;
  t.callChecker(argc);
  Exception::tryCatch(&Test::checkArgCount, &t, argc);
  std::cout << "This line will get executed, once stack is unwinded!" << std::endl;
  return 0;
}
