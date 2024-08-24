#include <ErrorHandler.hpp>
#include <Logger.hpp>

class TestClass : public ErrorHandler {

  /* public: */
  /*   static void test(int argc) { throw ServerException::ArgCount; } */
};

static void test(int argc) {
  if (argc) {
    throw ServerException::ArgCount;
  }
}

int main(int argc, char **argv) {
  (void)argv;
  ErrorHandler e;
  /* TestClass::test(argc); */
  /* TestClass::safeCall(test(argc)); */
  ErrorHandler::safeCall(&test, argc);
  return 0;
}
