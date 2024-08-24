#include <Exception.hpp>

class Test {
public:
  void callChecker(int argc) { privateFn(argc); };

  void checkArgCount(int argc) {
    if (argc > 1)
      throw ErrorCode::ArgCount;
  }

private:
  void privateFn(int argc) {
    Exception::tryCatch(&Test::checkArgCount, this, argc);
  }
};

int main(int argc, char **argv) {
  (void)argv;

  Test t;
  t.checkArgCount(argc);
  t.callChecker(argc);
  Exception::tryCatch(&Test::checkArgCount, &t, argc);
  return 0;
}
