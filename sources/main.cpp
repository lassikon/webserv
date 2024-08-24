#include <Exception.hpp>
#include <Logger.hpp>

class Test {
public:
  void callChecker(int argc) { privateFn(argc); };
  void checkArgCount(int argc) {
    if (argc)
      throw Exception("error");
  }

private:
  void privateFn(int argc) {
    Exception::tryCatch(&Test::checkArgCount, this, argc);
  }
};

int main(int argc, char **argv) {
  (void)argv;

  Test t;
  t.callChecker(argc);
  Exception::tryCatch(&Test::checkArgCount, &t, argc);
  /* LOG_INFO("Info from", 36.0f, "asdfasdf", 42); */
  return 0;
}
