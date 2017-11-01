#include "StringHelpers.hpp"
#include <sstream>

namespace StringHelpers
{
    std::vector<std::string> split(std::string string, char delimiter)
    {
        std::vector<std::string> result;

        std::stringstream stream(string);

        while (stream.good())
        {
            std::string subString;
            std::getline(stream, subString, delimiter);
            result.push_back(subString);
        }

        return result;
    }
}
