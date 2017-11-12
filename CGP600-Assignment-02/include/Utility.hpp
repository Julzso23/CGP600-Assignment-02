#pragma once

#include <vector>

namespace Utility
{
    // Split a string into an array of strings by a delimiter
    std::vector<std::string> split(std::string input, char delimiter);

    // Trim the leading and ending whitespace from a string
    std::string trim(std::string input);

    // Clamp a value between two other values
    template <typename T>
    T clamp(T value, T minimum, T maximum)
    {
        if (value < minimum)
        {
            return minimum;
        }

        if (value > maximum)
        {
            return maximum;
        }

        return value;
    }

    // Return the sign (+/-) of a value
    template <typename T>
    T sign(T value)
    {
        if (value < (T)0)
        {
            return (T)-1;
        }

        return (T)1;
    }

#undef max
#undef min

    // Return largest of the two values
    template <typename T>
    T max(T a)
    {
        return a;
    }

    template <typename T>
    T max(T a, T b)
    {
        if (a > b)
        {
            return a;
        }

        return b;
    }

    template <typename T, typename ... Numbers>
    T max(T a, T b, const Numbers&... others)
    {
        return max(max(a, b), others...);
    }

    // Return smallest of the two values
    template <typename T>
    T min(T a)
    {
        return a;
    }

    template <typename T>
    T min(T a, T b)
    {
        if (a < b)
        {
            return a;
        }

        return b;
    }

    template <typename T, typename ... Numbers>
    T min(T a, T b, const Numbers&... others)
    {
        return min(min(a, b), others...);
    }
}
