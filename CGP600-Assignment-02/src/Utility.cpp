#include "Utility.hpp"
#include <sstream>

namespace Utility
{
    std::vector<std::string> split(std::string input, char delimiter)
    {
        std::vector<std::string> result;

        std::stringstream stream(input);

        while (stream.good())
        {
            std::string subString;
            std::getline(stream, subString, delimiter);
            result.push_back(subString);
        }

        return result;
    }

    std::string trim(std::string input)
    {
        // Find the first character that wasn't whitespace
        const std::size_t start = input.find_first_not_of(" \t");

        // If the whole string was whitespace
        if (start == std::string::npos)
        {
            return "";
        }

        // Find the last character that wasn't whitespace
        const std::size_t end = input.find_last_not_of(" \t");
        const std::size_t range = end - start + 1;

        // Cut the whitespace from the string
        return input.substr(start, range);
    }
}
