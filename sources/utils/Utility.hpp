#pragma once

#include <string>
#include <iostream>

class Utility
{
    public:
        static std::string trimWhitespaces(std::string & line);
        static std::string trimComments(std::string & line);
};