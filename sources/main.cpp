#include "./config/config.hpp"

int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cerr << "Error: Invalid number of arguments" << std::endl;
        return 1;
    }
    try
    {
        Config config(argv[1]);
        config.printServerConfig();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return 0;
}
