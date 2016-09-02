#pragma once

#include <cstdio>
#include <string>
#include <sstream>

namespace DAVA
{
namespace Assert
{

static const size_t appendBufferSize = 256;

// String of all available format specifiers for easy use with std::find
static const std::string allFormatChars = "cCdiouxXeEfgGaAnpsS";

// Functions to check if a printf-style format specifier matches the argument
// All of them check the last symbol of a format string since it defines its type (like "%4.5f", "%d" etc.)
// TODO: enum support

template<typename T>
bool CheckFormatParameter(const std::string& format, const T param)
{
    static_assert(false, "CheckFormatParameter is not defined for this type");
}

template<>
inline bool CheckFormatParameter(const std::string& format, const char param)
{
    const char typeFieldCharacter = format.back();
    return (typeFieldCharacter == 'c' || typeFieldCharacter == 'C');
}

template<>
inline bool CheckFormatParameter(const std::string& format, const int param)
{
    const char typeFieldCharacter = format.back();
    return (typeFieldCharacter == 'd' || typeFieldCharacter == 'i' ||
            typeFieldCharacter == 'x' || typeFieldCharacter == 'X');
}

template<>
inline bool CheckFormatParameter(const std::string& format, const unsigned int param)
{
    const char typeFieldCharacter = format.back();
    return (typeFieldCharacter == 'o' || typeFieldCharacter == 'u');
}

template<>
inline bool CheckFormatParameter(const std::string& format, const float param)
{
    const char typeFieldCharacter = format.back();
    return (typeFieldCharacter == 'e' || typeFieldCharacter == 'E' ||
            typeFieldCharacter == 'f' || typeFieldCharacter == 'g' ||
            typeFieldCharacter == 'G' || typeFieldCharacter == 'a' ||
            typeFieldCharacter == 'A');
}

template<>
inline bool CheckFormatParameter(const std::string& format, void* const param)
{
    const char typeFieldCharacter = format.back();
    return (typeFieldCharacter == 'p');
}

inline bool CheckFormatParameter(const std::string& format, const char* param)
{
    const char typeFieldCharacter = format.back();
    return (typeFieldCharacter == 's' || typeFieldCharacter == 'S');
}

/// Appends formatted value into specified string stream
template<typename T>
inline void AppendFormat(std::ostringstream& stringStream, const std::string& format, T value)
{
    // Append error string if format and value do no match
    if (!CheckFormatParameter(format, value))
    {
        stringStream << "<wrong arg>";
    }
    else
    {
        char buffer[appendBufferSize];
        std::sprintf(buffer, format.c_str(), value);
        stringStream << buffer;
    }    
}

/// Returns length of a format specifier (assuming it starts on formatString[0])
inline int GetFormatSpecifierLength(const char* formatString)
{
    int resultLength = 0;

    while (*formatString)
    {
        const char currentChar = *formatString;
        ++resultLength;

        if (std::find(allFormatChars.begin(), allFormatChars.end(), currentChar) != allFormatChars.end())
        {
            break;
        }

        ++formatString;
    }
    
    return resultLength;
}

/// Base case for SafeFormat, just appends string to specified stringstream
inline void SafeFormat(std::ostringstream& output, const char* string)
{
    while (*string)
    {
        output << (*string);
        ++string;
    }
}

/// Safely formats string with specified arguments
/// Using unsupported types will end up with compiler errors
template<typename FirstArg, typename... RestArgs>
void SafeFormat(std::ostringstream& output, const char* formatString, FirstArg firstArg, RestArgs... restArgs)
{
    while (*formatString)
    {
        const char currentChar = *formatString;

        if (currentChar == '%')
        {
            // Handle %% special case
            if (*(formatString + 1) == '%')
            {
                ++formatString;
            }
            else
            {
                // Extract format specifier string
                const int length = GetFormatSpecifierLength(formatString);
                std::string formatSpecifier = std::string(formatString, length);

                // Format and append to output
                AppendFormat(output, formatSpecifier, firstArg);

                // Skip handled specifier
                formatString += length;

                // Recursively pass the rest of the string
                SafeFormat(output, formatString, restArgs...);

                break;
            }
        }

        // Just append current symbol sincce it's not apart of format specifier
        output << currentChar;
        ++formatString;
    }
}

template<typename... Args>
std::string SafeFormat(const char* formatString, Args... args)
{
    if (formatString == nullptr)
    {
        return std::string("");
    }

    std::ostringstream resultStream;
    SafeFormat(resultStream, formatString, args...);
    return resultStream.str();
}

}
}