#include "NGTCmdLineParser.h"

#include <locale>
#include <codecvt>

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
} // namespace NGTLayer
