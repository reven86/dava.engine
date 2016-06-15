#include "NGTCmdLineParser.h"

#include <locale>
#include <codecvt>
#include <algorithm>

namespace NGTLayer
{
NGTCmdLineParser::NGTCmdLineParser(int argc_, char** argv_)
    : m_argc(argc_)
    , m_argv(argv_)
{
}

int NGTCmdLineParser::argc() const
{
    return m_argc;
}

char** NGTCmdLineParser::argv() const
{
    return m_argv;
}

bool NGTCmdLineParser::getFlag(const char* arg) const
{
    size_t argLen = ::strlen(arg);
    for (int i = 0; i < m_argc; ++i)
    {
        if (::strlen(m_argv[i]) == argLen &&
            ::strncmp(m_argv[i], arg, argLen) == 0)
        {
            return true;
        }
    }
    return false;
}

const char* NGTCmdLineParser::getParam(const char* arg) const
{
    size_t argLen = ::strlen(arg);
    auto iter = std::find_if(additionalParams.begin(), additionalParams.end(), [arg, argLen](const std::pair<std::string, std::string>& p)
                             {
                                 return ::strncmp(p.first.c_str(), arg, argLen) == 0;
                             });

    if (iter != additionalParams.end())
    {
        return iter->second.c_str();
    }

    for (int i = 0; i < m_argc - 1; ++i)
    {
        if (::strlen(m_argv[i]) == argLen &&
            ::strncmp(m_argv[i], arg, argLen) == 0)
        {
            return m_argv[i + 1];
        }
    }
    return nullptr;
}

std::string NGTCmdLineParser::getParamStr(const char* arg) const
{
    auto param = getParam(arg);
    if (param != nullptr)
    {
        return param;
    }
    return "";
}

std::wstring NGTCmdLineParser::getParamStrW(const char* arg) const
{
    auto param = getParam(arg);
    if (param != nullptr)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(param);
    }
    return L"";
}

void NGTCmdLineParser::addParam(std::string&& key, std::string&& value)
{
    additionalParams.emplace_back(std::move(key), std::move(value));
}

} // namespace NGTLayer
