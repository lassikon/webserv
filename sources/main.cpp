#include <Exception.hpp>

class Test {
public:
  void func1(int argc) {
    if (argc)
      throw ErrorCode::ArgCount;
  }

  void func3(int argc) { Exception::tryCatch(&Test::func1, this, argc); }

  void callme(int argc) { func2(argc); };

private:
  void func2(int argc) { Exception::tryCatch(&Test::func1, this, argc); }
};

int main(int argc, char **argv) {
  (void)argv;

  Test t;
  t.func3(argc);
  t.callme(argc);
  Exception::tryCatch(&Test::func1, &t, argc);
  return 0;
}
