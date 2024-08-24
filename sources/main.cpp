#include <Exception.hpp>

class Test {
public:
  void callChecker(int argc) { privateFunc(argc); };
  void checkArgCount(int argc) {
    if (argc > 1)
      THROW_WARN(ERR_MSG_USAGE);
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
  return 0;
}
