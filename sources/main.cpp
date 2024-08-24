#include <Exception.hpp>
#include <stdexcept>

class Test {
public:
  void callChecker(int argc) { func2(argc); };
  void checkArgCount(int argc) {
    if (argc)
      /* throw std::runtime_error("runtime_error"); */
      throw ErrorCode::ArgCount;
  }

private:
  void func2(int argc) {
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
