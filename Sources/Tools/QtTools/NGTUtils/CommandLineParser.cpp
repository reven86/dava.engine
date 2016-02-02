/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "CommandLineParser.h"

#include <locale>
#include <codecvt>

CommandLineParser::CommandLineParser(int argc_, char** argv_)
    : m_argc(argc_)
    , m_argv(argv_)
{
}

int CommandLineParser::argc() const
{
    return m_argc;
}

char** CommandLineParser::argv() const
{
    return m_argv;
}

bool CommandLineParser::getFlag(const char* arg) const
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

const char* CommandLineParser::getParam(const char* arg) const
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

std::string CommandLineParser::getParamStr(const char* arg) const
{
    auto param = getParam(arg);
    if (param != nullptr)
    {
        return param;
    }
    return "";
}

std::wstring CommandLineParser::getParamStrW(const char* arg) const
{
    auto param = getParam(arg);
    if (param != nullptr)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(param);
    }
    return L"";
}
