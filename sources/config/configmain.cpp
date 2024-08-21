#include "config.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Error: Invalid number of arguments" << std::endl;
        return 1;
    }
    Config config(argv[1]);
    config.printServerConfig();
    return 0;
}