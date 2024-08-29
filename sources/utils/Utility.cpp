#include <Utility.hpp>

std::string    Utility::trimWhitespaces(std::string & line)
{
    size_t pos = line.find_first_not_of(" \t");
    if (pos != std::string::npos)
        line = line.substr(pos);
    pos = line.find_last_not_of(" \t");
    if (pos != std::string::npos)
        line = line.substr(0, pos + 1);
    return line;
}

std::string    Utility::trimComments(std::string & line)
{
    size_t pos = line.find("#");
    if (pos != std::string::npos)
        line = line.substr(0, pos);
    return line;
}